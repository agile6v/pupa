/*
 * Copyright (C) agile6v
 */

#include "pupa_cache.h"

static int pupa_cache_item_compare(const void *p1, const void *p2);
static void pupa_cache_item_make_mirror(pupa_ctx_t *ctx);
static int pupa_cache_item_add(pupa_ctx_t *ctx,
                               pupa_cache_item_t *cache_item,
                               pupa_str_t *key, pupa_str_t *value);
static int pupa_cache_item_replace(pupa_ctx_t *ctx,
                                   pupa_cache_item_t *cache_item,
                                   pupa_str_t *value);
static int _pupa_cache_item_compare(void *p1, const void *p2,
                                    const void *arg);
static int pupa_cache_value_compaction(pupa_ctx_t *ctx,
                                       pupa_str_t *value, char **address);
static int pupa_cache_key_compaction(pupa_ctx_t *ctx,
                                     pupa_str_t *key, char **address);


int32_t pupa_cache_init(pupa_cache_hdr_t *cache_hdr, int key_count)
{
    int32_t  offset;

    offset = sizeof(pupa_cache_hdr_t);

    //  initialize cache item section
    cache_hdr->item_section.size = key_count;
    cache_hdr->item_section.used = 0;
    cache_hdr->item_section.id = PUPA_CACHE_SECTION_ONE;
    cache_hdr->item_section.sec1_offset = offset;
    offset += key_count * sizeof(pupa_cache_item_t);
    cache_hdr->item_section.sec2_offset = offset;
    offset += key_count * sizeof(pupa_cache_item_t);

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

    return offset;
}


int pupa_cache_get(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value)
{
    char                         *p_value_addr;
    pupa_cache_item_t            *p_cache_item;
    pupa_cache_section_t         *p_item_section;
    pupa_cache_item_wrapper_t     cache_item_wrapper;

    p_value_addr = PUPA_CACHE_GET_ADDR(ctx->cache_hdr,
                                       ctx->cache_hdr->value_section);

    cache_item_wrapper.ctx = ctx;
    cache_item_wrapper.key_section_offset =
        PUPA_CACHE_GET_KEY_OFFSET(ctx->cache_hdr->key_section);
    cache_item_wrapper.cache_item.key_offset = (int32_t) (key->data -
        PUPA_CACHE_GET_ADDR(ctx->cache_hdr, ctx->cache_hdr->key_section));
    cache_item_wrapper.cache_item.key_len = key->len + 1;

    p_item_section = &ctx->cache_hdr->item_section;

    p_cache_item = bsearch(&cache_item_wrapper.cache_item,
                           ctx->cache_items,
                           p_item_section->used,
                           sizeof(pupa_cache_item_t),
                           pupa_cache_item_compare);
    if (p_cache_item == NULL) {
        DEBUG_LOG("Not found key: %.*s", key->len, key->data);
        return PUPA_NOT_FOUND;
    }

    if (p_cache_item->key_len == 0) {
        DEBUG_LOG("The length of the key (%.*s) is invalid.",
                  key->len, key->data);
        return PUPA_ERROR;
    }

    value->data = p_value_addr + p_cache_item->value_offset;
    value->len = p_cache_item->value_len;

    return PUPA_OK;
}


int pupa_cache_set(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value)
{
    int                          ret;
    pupa_cache_item_t             *p_cache_item;
    pupa_cache_section_t          *p_item_section, *p_key_section;
    pupa_cache_item_wrapper_t      cache_item_wrapper;

    //  generate the mirror of the cache items
    pupa_cache_item_make_mirror(ctx);

    p_item_section = &ctx->cache_hdr->item_section;
    p_key_section  = &ctx->cache_hdr->key_section;

    cache_item_wrapper.ctx = ctx;
    cache_item_wrapper.key_section_offset =
        PUPA_CACHE_GET_KEY_OFFSET(ctx->cache_hdr->key_section);
    cache_item_wrapper.cache_item.key_offset = (int32_t) (key->data -
        PUPA_CACHE_GET_ADDR(ctx->cache_hdr, ctx->cache_hdr->key_section));
    cache_item_wrapper.cache_item.key_len = key->len + 1;

    p_cache_item = bsearch(&cache_item_wrapper.cache_item,
                           ctx->cache_items_mirror,
                           p_item_section->used,
                           sizeof(pupa_cache_item_t),
                           pupa_cache_item_compare);
    if (p_cache_item == NULL) {
        DEBUG_LOG("Add key : %.*s - %.*s",
                  key->len, key->data, value->len, value->data);
        ret = pupa_cache_item_add(ctx,
                                  ctx->cache_items_mirror +
                                  p_item_section->used, key, value);
    } else {
        DEBUG_LOG("Replace key(%.*s) with (%.*s)",
                  key->len, key->data, value->len, value->data);
        ret = pupa_cache_item_replace(ctx, p_cache_item, value);
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
        cache_item_wrapper.key_section_offset =
            PUPA_CACHE_GET_KEY_MIRROR_OFFSET(ctx->cache_hdr->item_section);

#if (_PUPA_DARWIN)
        qsort_r(ctx->cache_items_mirror,
                ctx->cache_hdr->item_section.used,
                sizeof(pupa_cache_item_t),
                &cache_item_wrapper, _pupa_cache_item_compare);
#else
        qsort_r(ctx->cache_items_mirror,
                ctx->cache_hdr->item_section.used,
                sizeof(pupa_cache_item_t),
                _pupa_cache_item_compare, &cache_item_wrapper);
#endif
        p_item_section->id = (p_item_section->id == PUPA_CACHE_SECTION_ONE) ? \
                             PUPA_CACHE_SECTION_TWO : p_item_section->id;
        p_item_section->used++;
    }

    return PUPA_OK;
}


int pupa_cache_del(pupa_ctx_t *ctx, pupa_str_t *key)
{
    return PUPA_OK;
}


static int pupa_cache_item_add(pupa_ctx_t *ctx,
                               pupa_cache_item_t *cache_item,
                               pupa_str_t *key, pupa_str_t *value)
{
    char             *p = NULL;
    int               ret;
    pupa_cache_hdr_t *cache_hdr = ctx->cache_hdr;

    if (cache_hdr->item_section.used == cache_hdr->item_section.size) {
        return PUPA_OVERFLOW;
    }

    if ((cache_hdr->key_section.used + (key->len + 1)) >
            cache_hdr->key_section.size)
    {
        ret = pupa_cache_key_compaction(ctx, key, &p);
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

    if ((cache_hdr->value_section.used + (value->len + 1)) >
        cache_hdr->value_section.size)
    {
        DEBUG_LOG("Start to compact the value section.");
        ret = pupa_cache_value_compaction(ctx, value, &p);
        if (ret != PUPA_OK) {
            return ret;
        }
    }

    if (p == NULL) {
        p = PUPA_CACHE_GET_FREE_ADDR(cache_hdr, cache_hdr->value_section);
    }

    cache_item->value_offset = cache_hdr->value_section.used;
    cache_item->value_len = value->len + 1;

    memcpy(p, value->data, value->len);
    p += value->len;
    *p = '\0';
    
    cache_hdr->value_section.used += cache_item->value_len;

    return PUPA_OK;
}


static int pupa_cache_item_replace(pupa_ctx_t *ctx,
                                   pupa_cache_item_t *cache_item,
                                   pupa_str_t *value)
{
    int               ret;
    char             *p = NULL;
    pupa_cache_hdr_t *cache_hdr = ctx->cache_hdr;

    if ((cache_hdr->value_section.used + (value->len + 1)) >
        cache_hdr->value_section.size)
    {
        ret = pupa_cache_value_compaction(ctx, value, &p);
        if (ret != PUPA_OK) {
            return ret;
        }
    }

    if (p == NULL) {
        p = PUPA_CACHE_GET_FREE_ADDR(cache_hdr, cache_hdr->value_section);
    }

    //  copy the data of the value to the cache
    cache_item->value_offset = cache_hdr->value_section.used;
    cache_item->value_len = value->len + 1;

    p = memcpy(p, value->data, value->len);
    *p = '\0';

    cache_hdr->value_section.used += cache_item->value_len;
    
    return PUPA_OK;
}


static int _pupa_cache_item_compare(void *p1, const void *p2,
                                      const void *arg)
{
    int                      ret;
    int32_t                  key_section_offset;
    char                    *p_cache_hdr;
    pupa_cache_item_wrapper_t *p_cache_item_wrapper;

    ret = ((pupa_cache_item_t *) p1)->key_len - ((pupa_cache_item_t *)p2)->key_len;
    if (ret != 0) {
        return ret;
    }

    p_cache_item_wrapper = (pupa_cache_item_wrapper_t *) arg;

    p_cache_hdr = (char *) p_cache_item_wrapper->ctx->cache_hdr;
    key_section_offset = p_cache_item_wrapper->key_section_offset;

    return memcmp(p_cache_hdr + key_section_offset +
                  ((pupa_cache_item_t *) p1)->key_offset,
                  p_cache_hdr + key_section_offset +
                  ((pupa_cache_item_t *) p2)->key_offset,
                  ((pupa_cache_item_t *) p1)->key_len);
}


static int pupa_cache_item_compare(const void *p1, const void *p2)
{
    pupa_cache_item_wrapper_t *p_cache_item_wrapper;

    p_cache_item_wrapper = (pupa_cache_item_wrapper_t *)
            ((char *) p1 - offsetof(pupa_cache_item_wrapper_t, cache_item));

    return _pupa_cache_item_compare((void *) p1, p2, p_cache_item_wrapper);
}


static int pupa_cache_value_compaction(pupa_ctx_t *ctx,
                                       pupa_str_t *value, char **address)
{
    int                i;
    char              *p;
    size_t             used_size;
    pupa_cache_item_t *p_cache_item;

    p = PUPA_CACHE_GET_MIRROR_ADDR(ctx->cache_hdr,
                                   ctx->cache_hdr->value_section);

    used_size = 0;
    for (i = 0; i < ctx->cache_hdr->item_section.used; i++) {
        p_cache_item = &ctx->cache_items_mirror[i];
        memcpy(p + used_size, p + p_cache_item->value_offset,
               p_cache_item->value_len);
        p_cache_item->value_offset = used_size;
        used_size += p_cache_item->value_len;
    }

    if ((value->len + 1 + used_size) > ctx->cache_hdr->value_section.size) {
        return PUPA_OVERFLOW;
    }

    *address = p;
    ctx->cache_hdr->value_section.used = used_size;

    return PUPA_OK;
}


static int pupa_cache_key_compaction(pupa_ctx_t *ctx,
                                     pupa_str_t *key, char **address)
{
    int i;
    char *p;
    size_t used_size;
    pupa_cache_item_t *p_cache_item;

    p = PUPA_CACHE_GET_MIRROR_ADDR(ctx->cache_hdr,
                                   ctx->cache_hdr->key_section);

    used_size = 0;
    for (i = 0; i < ctx->cache_hdr->item_section.used; i++) {
        p_cache_item = &ctx->cache_items_mirror[i];
        memcpy(p + used_size, p + p_cache_item->key_offset,
               p_cache_item->key_len);
        p_cache_item->key_offset = used_size;
        used_size += p_cache_item->key_len;
    }

    if ((key->len + 1 + used_size) > ctx->cache_hdr->key_section.size) {
        return PUPA_OVERFLOW;
    }

    *address = p;
    ctx->cache_hdr->key_section.used = used_size;

    return PUPA_OK;
}


static void pupa_cache_item_make_mirror(pupa_ctx_t *ctx)
{
    pupa_cache_item_t     *p_cache_items;

    p_cache_items = (pupa_cache_item_t *)
            PUPA_CACHE_GET_MIRROR_ADDR(ctx->cache_hdr,
                                       ctx->cache_hdr->item_section);

    if (ctx->cache_hdr->item_section.used) {
        memcpy(p_cache_items, ctx->cache_items,
               sizeof(pupa_cache_item_t) * ctx->cache_hdr->item_section.used);
    }

    ctx->cache_items_mirror = p_cache_items;
}


int pupa_cache_stats(pupa_ctx_t *ctx, pupa_cache_stats_t *stat)
{
    return PUPA_OK;
}


int pupa_cache_fini(pupa_ctx_t *ctx)
{
    int ret;

    ret = pupa_shm_fini(ctx);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}