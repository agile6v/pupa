/*
 * Copyright (C) agile6v
 */

#include "pupa_cache.h"
#include "pupa_config.h"

static int pupa_cache_item_compare(const void *p1, const void *p2);
static void pupa_cache_item_make_mirror(pupa_cache_hdr *cache_hdr);

int32_t pupa_cache_init(pupa_cache_hdr *cache_hdr, int key_count)
{
    int32_t  offset;

    offset = sizeof(pupa_cache_hdr);

    //  initialize cache item section
    cache_hdr->item_section.size = key_count;
    cache_hdr->item_section.used = 0;
    cache_hdr->item_section.id = PUPA_CACHE_SECTION_ONE;
    cache_hdr->item_section.sec1_offset = offset;
    offset += key_count * sizeof(pupa_cache_item);
    cache_hdr->item_section.sec2_offset = offset;
    offset += key_count * sizeof(pupa_cache_item);

    //  initialize cache key section
    cache_hdr->key_section.size = PUPA_KEY_AVERAGE_LEN * key_count;
    cache_hdr->key_section.used = 0;
    cache_hdr->key_section.id = PUPA_CACHE_SECTION_ONE;
    cache_hdr->key_section.sec1_offset = offset;
    offset += key_count * PUPA_KEY_AVERAGE_LEN;
    cache_hdr->key_section.sec2_offset = offset;
    offset += key_count * PUPA_KEY_AVERAGE_LEN;

    //  initialize cache value section
    cache_hdr->value_section.size = PUPA_VALUE_AVERAGE_LEN * key_count;
    cache_hdr->value_section.used = 0;
    cache_hdr->value_section.id = PUPA_CACHE_SECTION_ONE;
    cache_hdr->value_section.sec1_offset = offset;
    offset += key_count * PUPA_VALUE_AVERAGE_LEN;
    cache_hdr->value_section.sec2_offset = offset;
    offset += key_count * PUPA_VALUE_AVERAGE_LEN;

    //  By default, using item section one
    cache_hdr->cache_items = (pupa_cache_item *) ((char *) cache_hdr +
            cache_hdr->item_section.sec1_offset);

    return offset;
}


int pupa_cache_del(pupa_ctx *ctx, pupa_str_t *key)
{
    return PUPA_OK;
}


int pupa_cache_get(pupa_ctx *ctx, pupa_str_t *key, pupa_str_t *value)
{
    char                       *p;
    pupa_cache_item            *p_cache_item;
    pupa_cache_section         *p_key_section;
    pupa_cache_section         *p_item_section;
    pupa_cache_item_wrapper     cache_item_wrapper;

    p_key_section  = &ctx->cache_hdr->key_section;

    cache_item_wrapper.ctx = ctx;
    cache_item_wrapper.key_section_offset =
            (p_key_section->id == PUPA_CACHE_SECTION_ONE) ? \
             p_key_section->sec1_offset : \
             p_key_section->sec2_offset;
    cache_item_wrapper.cache_item.key_offset =
            key->data - cache_item_wrapper.key_section_offset;
    cache_item_wrapper.cache_item.key_len = key->len;

    p_item_section = &ctx->cache_hdr->item_section;

    p_cache_item = bsearch(&cache_item_wrapper.cache_item,
                           &ctx->cache_hdr->cache_items,
                           p_item_section.used,
                           sizeof(pupa_cache_item),
                           pupa_cache_item_compare);
    if (p_cache_item == NULL) {
        return PUPA_NOT_FOUND;
    }

    if (p_cache_item->key_len == 0) {
        //  TODO: ERROR LOG
        return PUPA_ERROR;
    }


    if (ctx->cache_hdr->value_section.id == PUPA_CACHE_SECTION_ONE) {
        p = (char *) ctx->cache_hdr + ctx->cache_hdr->value_section.sec1_offset +
            cache_item->value_offset + sizeof(int16_t);
    } else {
        p = (char *) ctx->cache_hdr + ctx->cache_hdr->value_section.sec2_offset +
            cache_item->value_offset + sizeof(int16_t);
    }

    value->data = p;
    value->len = p_cache_item->value_len;

    return PUPA_OK;
}


int pupa_cache_set(pupa_ctx *ctx, pupa_str_t *key, pupa_str_t *value)
{
    int                          ret;
    pupa_cache_item             *p_cache_item;
    pupa_cache_item             *p_next_cache_items, *p_last_cache_items;
    pupa_cache_section          *p_item_section, *p_key_section;
    pupa_cache_item_wrapper      cache_item_wrapper;

    //  generate the mirror of the cache items
    pupa_cache_item_make_mirror(ctx->cache_hdr);

    p_item_section = &ctx->cache_hdr->item_section;
    p_key_section  = &ctx->cache_hdr->key_section;

    cache_item_wrapper.ctx = ctx;
    cache_item_wrapper.key_section_offset =
            (p_key_section->id == PUPA_CACHE_SECTION_ONE) ? \
             p_key_section->sec1_offset : \
             p_key_section->sec2_offset;
    cache_item_wrapper.cache_item.key_offset =
            key->data - cache_item_wrapper.key_section_offset;
    cache_item_wrapper.cache_item.key_len = key->len;

    find_cache_item.key_offset = key->data - p_key_section;

    p_cache_item = bsearch(&cache_item_wrapper.cache_item,
                           &ctx->cache_hdr->cache_items_mirror,
                           p_item_section.used,
                           sizeof(pupa_cache_item),
                           pupa_cache_item_compare);
    if (p_cache_item == NULL) {
        ret = pupa_cache_item_add(ctx->cache_hdr,
                                  ctx->cache_hdr->cache_items_mirror +
                                  p_item_section.used, key, value);
    } else {
        ret = pupa_cache_item_replace(ctx->cache_hdr, p_cache_item, value);
    }

    if (ret != PUPA_OK) {
        return ret;
    }

    ret = pupa_shm_sync(ctx);
    if (ret != PUPA_OK) {
        return ret;
    }

    // Switch the cache item section if this key doesn't exist
    if (p_cache_item == NULL) {
        p_item_section->id = (p_item_section->id == PUPA_CACHE_SECTION_ONE) ? \
                             PUPA_CACHE_SECTION_TWO : p_item_section->id;
        p_item_section->used++;
    }

    return PUPA_OK;
}


static int pupa_cache_item_del()
{
    return PUPA_OK;
}


static int pupa_cache_item_add(pupa_cache_hdr *cache_hdr,
                               pupa_cache_item *cache_item,
                               pupa_str_t *key, pupa_str_t *value)
{
    char    *p = NULL;
    int      ret;
    int16_t *p_value_size;

    if (cache_hdr->item_section.used == cache_hdr->item_section.size) {
        return PUPA_OVERFLOW;
    }

    if ((cache_hdr->key_section.used + (key->len + 1)) >
            cache_hdr->key_section.size)
    {
        ret = pupa_cache_key_compaction(cache_hdr, key, &p);
        if (ret != PUPA_OK) {
            return ret;
        }
    }

    if (p == NULL) {
        p = PUPA_CACHE_GET_FREE_ADDR(cache_hdr, cache_hdr->key_section);
    }

    //  copy the data of the key to the cache
    cache_item->key_len = key->len + 1;
    cache_item->key_offset = cache_hdr->key_section.used;

    memcpy(p, key->data, key->len);
    p += key->len;
    *p = '\0';
    cache_hdr->key_section.used += cache_item->key_len;

    //  copy the data of the value to the cache
    p = NULL;

    if ((cache_hdr->value_section.used + (value->len + 1 + sizeof(int16_t))) >
        cache_hdr->value_section.size)
    {
        ret = pupa_cache_value_compaction(cache_hdr, value, &p);
        if (ret != PUPA_OK) {
            return ret;
        }
    }

    if (p == NULL) {
        p = PUPA_CACHE_GET_FREE_ADDR(cache_hdr, cache_hdr->value_section);
    }

    cache_item->value_offset = cache_hdr->value_section.used;
    cache_item->value_len = sizeof(int16_t) + value->len + 1;

    p_value_size = (int16_t *) p;
    *p_value_size = value->len + 1;

    p = p + sizeof(int16_t);
    memcpy(p, value->data, value->len);
    p += value->len;
    *p = '\0';
    
    cache_hdr->value_section.used += cache_item->value_len;

    return PUPA_OK;
}


static int pupa_cache_item_replace(pupa_cache_hdr *cache_hdr,
                                   pupa_cache_item *cache_item,
                                   pupa_str_t *value)
{
    char    *p = NULL;
    int16_t *p_value_size;

    if ((cache_hdr->value_section.used + (value->len + 1 + sizeof(int16_t))) >
        cache_hdr->value_section.size)
    {
        ret = pupa_cache_value_compaction(cache_hdr, value, &p);
        if (ret != PUPA_OK) {
            return ret;
        }
    }

    if (p == NULL) {
        p = PUPA_CACHE_GET_FREE_ADDR(cache_hdr, cache_hdr->value_section);
    }

    //  copy the data of the value to the cache
    cache_item->value_offset = cache_hdr->value_section.used;
    cache_item->value_len = sizeof(int16_t) + value->len + 1;

    p_value_size = (int16_t *) p;
    *p_value_size = value->len + 1;

    p = p + sizeof(int16_t);
    p = memcpy(p, value->data, value->len);
    *p = '\0';

    cache_hdr->value_section.used += cache_item->value_len;
    
    return PUPA_OK;
}


static int _pupa_cache_item_compare(const void *p1, const void *p2,
                                      void *arg)
{
    int                      ret;
    int32_t                  key_section_offset;
    char                    *p_cache_hdr;
    pupa_cache_item_wrapper *p_cache_item_wrapper;

    ret = ((pupa_cache_item *) p1)->key_len - ((pupa_cache_item *)p2)->key_len;
    if (ret != 0) {
        return ret;
    }

    p_cache_item_wrapper = (pupa_cache_item_wrapper *) arg;

    p_cache_hdr = (char *) p_cache_item_wrapper->ctx->cache_hdr;
    key_section_offset = p_cache_item_wrapper->key_section_offset;

    return memcmp(p_cache_hdr + key_section_offset +
                  ((pupa_cache_item *) p1)->key_offset,
                  p_cache_hdr + key_section_offset +
                  ((pupa_cache_item *) p2)->key_offset,
                  ((pupa_cache_item *) p1)->key_len);
}


static int pupa_cache_item_compare(const void *p1, const void *p2)
{
    pupa_cache_item_wrapper *p_cache_item_wrapper;

    p_cache_item_wrapper = (pupa_cache_item_wrapper *)
            ((char *) p1 - offsetof(pupa_cache_item_wrapper, cache_item));

    return _pupa_cache_item_compare(p1, p2, p_cache_item_wrapper);
}


static int pupa_cache_value_compaction(pupa_cache_hdr *cache_hdr,
                                     pupa_str_t *value, char **address)
{
    int i;
    char *p;
    size_t used_size;
    pupa_cache_item *p_cache_items;
    pupa_cache_item *p_cache_item;

    p = PUPA_CACHE_GET_MIRROR_ADDR(cache_hr, cache_hdr->value_section);

    used_size = 0;
    for (i = 0; i < cache_hdr->item_section.used; i++) {
        p_cache_item = &cache_hdr->cache_items_mirror[i];
        memcpy(p + used_size, p_cache_item->value_offset,
               p_cache_item->value_len);
        p_cache_item->value_offset = used_size;
        used_size += p_cache_item->value_len;
    }

    if ((value->len + 1 + used_size) > cache_hdr->value_section.size) {
        return PUPA_OVERFLOW;
    }

    *address = p;

    return PUPA_OK;
}


static int pupa_cache_key_compaction(pupa_cache_hdr *cache_hdr,
                                     pupa_str_t *key, char **address)
{
    int i;
    char *p;
    size_t used_size;
    pupa_cache_item *p_cache_items;
    pupa_cache_item *p_cache_item;

    dp = PUPA_CACHE_GET_MIRROR_ADDR(cache_hr, cache_hdr->key_section);

    used_size = 0;
    for (i = 0; i < cache_hdr->item_section.used; i++) {
        p_cache_item = &cache_hdr->cache_items_mirror[i];
        memcpy(p + used_size, p_cache_item->key_offset,
               p_cache_item->key_len);
        p_cache_item->key_offset = used_size;
        used_size += p_cache_item->key_len;
    }

    if ((key->len + 1 + used_size) > cache_hdr->key_section.size) {
        return PUPA_OVERFLOW;
    }

    *address = p;

    return PUPA_OK;
}

static void pupa_cache_item_make_mirror(pupa_cache_hdr *cache_hdr)
{
    pupa_cache_item *p_cache_items;

    if (cache_hdr->item_section.id == PUPA_CACHE_SECTION_ONE) {
        p_cache_items = (pupa_cache_item *) ((char *) cache_hdr +
                cache_hdr->item_section.sec2_offset);
    } else {
        p_cache_items = (pupa_cache_item *) ((char *) cache_hdr +
                cache_hdr->item_section.sec1_offset);
    }

    memcpy(p_cache_items, cache_hdr->cache_items,
           sizeof(pupa_cache_item) * cache_hdr->item_section.used);

    cache_hdr->cache_items_mirror = p_cache_items;
}

