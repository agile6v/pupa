/*
 * Copyright (C) agile6v
 */

#include "pupa_cache.h"
#include "pupa_config.h"

static int pupa_cache_item_compare(const void *p1, const void *p2);

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

    return offset;
}


int pupa_cache_set(pupa_ctx *ctx, pupa_str_t *key, pupa_str_t *value)
{
    pupa_cache_item *p_cache_item;
    pupa_cache_item *p_next_cache_items, *p_last_cache_items;
    pupa_cache_item_wrapper cache_item_wrapper;

    if (ctx->cache_hdr->item_section.id == PUPA_CACHE_SECTION_ONE) {
        p_last_cache_items = (pupa_cache_item *) ((char *) ctx->cache_hdr +
                ctx->cache_hdr->item_section.sec1_offset);

        p_next_cache_items = (pupa_cache_item *) ((char *) ctx->cache_hdr +
                ctx->cache_hdr->item_section.sec2_offset);
    } else {
        p_last_cache_items = (pupa_cache_item *) ((char *) ctx->cache_hdr +
                ctx->cache_hdr->item_section.sec2_offset);

        p_next_cache_items = (pupa_cache_item *) ((char *) ctx->cache_hdr +
                ctx->cache_hdr->item_section.sec1_offset);
    }

    memcpy(p_next_cache_items, p_last_cache_items,
           sizeof(pupa_cache_item) * ctx->cache_hdr->item_section.used);

    cache_item_wrapper.key_section_offset =
            (ctx->cache_hdr->key_section.id == PUPA_CACHE_SECTION_ONE) ? \
            ctx->cache_hdr->key_section.sec1_offset : \
            ctx->cache_hdr->key_section.sec2_offset;
    cache_item_wrapper.cache_item.key_offset =
            key->data - cache_item_wrapper.key_section_offset;
    cache_item_wrapper.cache_item.key_len = key->len;

    find_cache_item.key_offset = key->data - ctx->cache_hdr->key_section;

    p_cache_item = bsearch(&cache_item_wrapper.cache_item,
                           &p_next_cache_items,
                           ctx->cache_hdr->item_section.used,
                           sizeof(pupa_cache_item),
                           pupa_cache_item_compare);
    if (p_cache_item == NULL) {
        //  TODO:   add cache item
        pupa_cache_item_add();
    } else {
        //  TODO:   replace cache item
        pupa_cache_item_replace();
    }

    return PUPA_OK;
}

static int pupa_cache_item_del()
{
    return PUPA_OK;
}


static pupa_cache_item *pupa_cache_item_add(pupa_cache_hdr *cache_hdr,
                                            pupa_cache_item *cache_item,
                                            pupa_str_t *key, pupa_str_t *value)
{
    char    *p;

    cache_item->key_len = key->len;
    cache_item->key_offset = cache_hdr->key_section.used;

    if (cache_hdr->key_section.id == PUPA_CACHE_SECTION_ONE) {
        p = (char *) cache_hdr + cache_hdr->key_section.sec2_offset +
                cache_item->key_offset;
    } else {
        p = (char *) cache_hdr + cache_hdr->key_section.sec1_offset +
                cache_item->key_offset;
    }

    memcpy(p, key->data, key->len);
    cache_hdr->key_section.used += (key->len + 1);

    return PUPA_OK;
}

static int pupa_cache_item_replace()
{
    return PUPA_OK;
}

static int pupa_cache_item_compare(const void *p1, const void *p2)
{
    int                      ret;
    char                    *p_cache_hdr;
    int32_t                  key_section_offset;
    pupa_cache_item_wrapper *p_cache_item_wrapper;

    ret = ((pupa_cache_item *) p1)->key_len - ((pupa_cache_item *)p2)->key_len;
    if (ret != 0) {
        return ret;
    }

    p_cache_item_wrapper = (pupa_cache_item_wrapper *)
            ((char *) p1 - offsetof(pupa_cache_item_wrapper, cache_item));

    p_cache_hdr = (char *) p_cache_item_wrapper->ctx->cache_hdr;
    key_section_offset = p_cache_item_wrapper->key_section_offset;

    return memcmp(p_cache_hdr + key_section_offset +
                         ((pupa_cache_item *) p1)->key_offset,
                 p_cache_hdr + key_section_offset +
                         ((pupa_cache_item *) p2)->key_offset,
                 ((pupa_cache_item *) p1)->key_len);
}

