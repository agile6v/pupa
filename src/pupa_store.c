/*
 * Copyright (C) agile6v
 */

#include "pupa_store.h"

#if (_PUPA_DARWIN)
static int _pupa_store_item_compare(void *p1, const void *p2, const void *arg);
#else
static int _pupa_store_item_compare(const void *p1, const void *p2, void *arg);
#endif
static int  pupa_store_item_compare(const void *p1, const void *p2);
static void pupa_store_item_make_mirror(pupa_ctx_t *ctx);
static int  pupa_store_item_add(pupa_ctx_t *ctx, pupa_store_item_t *store_item,
                                pupa_str_t *key, pupa_str_t *value);
static int  pupa_store_item_replace(pupa_ctx_t *       ctx,
                                    pupa_store_item_t *store_item,
                                    pupa_str_t *       value);
static int  pupa_store_value_compaction(pupa_ctx_t *ctx, pupa_str_t *value,
                                        char **address);
static int  pupa_store_key_compaction(pupa_ctx_t *ctx, pupa_str_t *key,
                                      char **address);

int32_t pupa_store_init(pupa_store_hdr_t *store_hdr, int key_count)
{
    int32_t offset;

    offset = sizeof(pupa_store_hdr_t);

    //  initialize cache item section
    store_hdr->item_section.size        = key_count;
    store_hdr->item_section.used        = 0;
    store_hdr->item_section.id          = PUPA_STORE_SECTION_ONE;
    store_hdr->item_section.sec1_offset = offset;
    offset += key_count * sizeof(pupa_store_item_t);
    store_hdr->item_section.sec2_offset = offset;
    offset += key_count * sizeof(pupa_store_item_t);

    //  initialize cache key section
    store_hdr->key_section.size        = PUPA_KEY_AVERAGE_LEN * key_count;
    store_hdr->key_section.used        = 0;
    store_hdr->key_section.id          = PUPA_STORE_SECTION_ONE;
    store_hdr->key_section.sec1_offset = offset;
    offset += key_count * PUPA_KEY_AVERAGE_LEN;
    store_hdr->key_section.sec2_offset = offset;
    offset += key_count * PUPA_KEY_AVERAGE_LEN;

    //  initialize cache value section
    store_hdr->value_section.size        = PUPA_VALUE_AVERAGE_LEN * key_count;
    store_hdr->value_section.used        = 0;
    store_hdr->value_section.id          = PUPA_STORE_SECTION_ONE;
    store_hdr->value_section.sec1_offset = offset;
    offset += key_count * PUPA_VALUE_AVERAGE_LEN;
    store_hdr->value_section.sec2_offset = offset;
    offset += key_count * PUPA_VALUE_AVERAGE_LEN;

    return offset;
}

int pupa_store_get(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value)
{
    pupa_store_item_t         *p_cache_item;
    pupa_store_section_t      *p_item_section;
    pupa_store_item_wrapper_t  store_item_wrapper;

    ctx->store_items = (pupa_store_item_t *) PUPA_STORE_GET_ADDR(
            ctx->store_hdr, ctx->store_hdr->item_section);

    store_item_wrapper.ctx = ctx;
    store_item_wrapper.key_offset =
            (int64_t) (key->data - (char *) ctx->store_hdr);
    store_item_wrapper.store_item.key_len = key->len + 1;

    p_item_section = &ctx->store_hdr->item_section;

    p_cache_item = bsearch(&store_item_wrapper.store_item, ctx->store_items,
                           p_item_section->used, sizeof(pupa_store_item_t),
                           pupa_store_item_compare);
    if (p_cache_item == NULL) {
        DEBUG_LOG("Not found key: %.*s", key->len, key->data);
        return PUPA_NOT_FOUND;
    }

    if (p_cache_item->key_len == 0) {
        DEBUG_LOG("The length of the key (%.*s) is invalid.", key->len,
                  key->data);
        return PUPA_ERROR;
    }

    value->data = (char *) ctx->store_hdr + p_cache_item->value_offset;
    value->len  = p_cache_item->value_len;

    return PUPA_OK;
}

int pupa_store_set(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value)
{
    int                       ret;
    pupa_store_item_t        *p_cache_item;
    pupa_store_section_t     *p_item_section;
    pupa_store_item_wrapper_t store_item_wrapper;

    //  generate the mirror of the cache items
    pupa_store_item_make_mirror(ctx);

    p_item_section = &ctx->store_hdr->item_section;

    store_item_wrapper.ctx = ctx;
    store_item_wrapper.key_offset =
            (int64_t) (key->data - (char *) ctx->store_hdr);
    store_item_wrapper.store_item.key_len = key->len + 1;

    p_cache_item = bsearch(&store_item_wrapper.store_item,
                           ctx->store_items_mirror, p_item_section->used,
                           sizeof(pupa_store_item_t), pupa_store_item_compare);
    if (p_cache_item == NULL) {
        DEBUG_LOG("Add key : %.*s - %.*s", key->len, key->data, value->len,
                  value->data);
        ret = pupa_store_item_add(
            ctx, ctx->store_items_mirror + p_item_section->used, key, value);
    } else {
        DEBUG_LOG("Replace key(%.*s) with (%.*s)", key->len, key->data,
                  value->len, value->data);
        ret = pupa_store_item_replace(ctx, p_cache_item, value);
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
        store_item_wrapper.key_offset = 0;
#if (_PUPA_DARWIN)
        qsort_r(ctx->store_items_mirror, ctx->store_hdr->item_section.used,
                sizeof(pupa_store_item_t), &store_item_wrapper,
                _pupa_store_item_compare);
#else
        qsort_r(ctx->store_items_mirror, ctx->store_hdr->item_section.used,
                sizeof(pupa_store_item_t), _pupa_store_item_compare,
                &store_item_wrapper);
#endif

        p_item_section->used++;
    }

    p_item_section->id =
            PUPA_STORE_GET_SEC_MIRROR_ID(ctx->store_hdr->item_section);

    return PUPA_OK;
}

int pupa_store_del(pupa_ctx_t *ctx, pupa_str_t *key)
{
    int                       ret;
    pupa_store_item_t *       p_cache_item;
    pupa_store_section_t *    p_item_section;
    pupa_store_item_wrapper_t store_item_wrapper;

    //  generate the mirror of the cache items
    pupa_store_item_make_mirror(ctx);

    p_item_section = &ctx->store_hdr->item_section;

    store_item_wrapper.ctx = ctx;
    store_item_wrapper.key_offset =
            (int64_t) (key->data - (char *) ctx->store_hdr);
    store_item_wrapper.store_item.key_len = key->len + 1;

    p_cache_item = bsearch(&store_item_wrapper.store_item,
                           ctx->store_items_mirror, p_item_section->used,
                           sizeof(pupa_store_item_t), pupa_store_item_compare);
    if (p_cache_item == NULL) {
        DEBUG_LOG("Not found key: %.*s", key->len, key->data);
        return PUPA_NOT_FOUND;
    }

    ret = pupa_shm_sync(ctx);
    if (ret != PUPA_OK) {
        return ret;
    }

    p_cache_item->key_len = ctx->store_hdr->key_section.size;

    store_item_wrapper.key_offset = 0;

#if (_PUPA_DARWIN)
    qsort_r(ctx->store_items_mirror, ctx->store_hdr->item_section.used,
            sizeof(pupa_store_item_t), &store_item_wrapper,
            _pupa_store_item_compare);
#else
    qsort_r(ctx->store_items_mirror, ctx->store_hdr->item_section.used,
            sizeof(pupa_store_item_t), _pupa_store_item_compare,
            &store_item_wrapper);
#endif

    p_item_section->id = (p_item_section->id == PUPA_STORE_SECTION_ONE)
                             ? PUPA_STORE_SECTION_TWO
                             : p_item_section->id;

    p_item_section->used--;

    return PUPA_OK;
}

static int pupa_store_item_add(pupa_ctx_t *ctx, pupa_store_item_t *store_item,
                               pupa_str_t *key, pupa_str_t *value)
{
    int               ret;
    size_t            offset;
    char             *p = NULL;
    pupa_store_hdr_t *store_hdr = ctx->store_hdr;

    if (store_hdr->item_section.used == store_hdr->item_section.size) {
        return PUPA_OVERFLOW;
    }

    if ((store_hdr->key_section.used + (key->len + 1)) >
        store_hdr->key_section.size)
    {
        DEBUG_LOG("Start to compact the key section.");

        offset = PUPA_STORE_GET_MIRROR_OFFSET(store_hdr->key_section);

        ret = pupa_store_key_compaction(ctx, key, &p);
        if (ret != PUPA_OK) {
            return ret;
        }
    } else {
        p = PUPA_STORE_GET_FREE_ADDR(store_hdr, store_hdr->key_section);
        offset = PUPA_STORE_GET_OFFSET(store_hdr->key_section);
    }

    //  copy the data of the key to the cache
    store_item->key_len    = key->len + 1;
    store_item->key_offset = offset + store_hdr->key_section.used;

    memcpy(p, key->data, key->len);
    p += key->len;
    *p = '\0';
    store_hdr->key_section.used += store_item->key_len;

    //  copy the data of the value to the cache
    p = NULL;

    if ((store_hdr->value_section.used + (value->len + 1)) >
        store_hdr->value_section.size)
    {
        DEBUG_LOG("Start to compact the value section.");

        offset = PUPA_STORE_GET_MIRROR_OFFSET(store_hdr->value_section);

        ret = pupa_store_value_compaction(ctx, value, &p);
        if (ret != PUPA_OK) {
            return ret;
        }
    } else {
        p = PUPA_STORE_GET_FREE_ADDR(store_hdr, store_hdr->value_section);
        offset = PUPA_STORE_GET_OFFSET(store_hdr->value_section);
    }

    store_item->value_offset = offset + store_hdr->value_section.used;
    store_item->value_len    = value->len + 1;

    memcpy(p, value->data, value->len);
    p += value->len;
    *p = '\0';

    store_hdr->value_section.used += store_item->value_len;

    return PUPA_OK;
}

static int pupa_store_item_replace(pupa_ctx_t        *ctx,
                                   pupa_store_item_t *store_item,
                                   pupa_str_t        *value)
{
    char             *p         = NULL;
    pupa_store_hdr_t *store_hdr = ctx->store_hdr;
    size_t            offset;
    int               ret;

    if ((store_hdr->value_section.used + (value->len + 1)) >
        store_hdr->value_section.size)
    {
        DEBUG_LOG("Start to compact the value section.");

        offset = PUPA_STORE_GET_MIRROR_OFFSET(store_hdr->value_section);

        ret = pupa_store_value_compaction(ctx, value, &p);
        if (ret != PUPA_OK) {
            return ret;
        }
    } else {
        p = PUPA_STORE_GET_FREE_ADDR(store_hdr, store_hdr->value_section);
        offset = PUPA_STORE_GET_OFFSET(store_hdr->value_section);
    }

    //  copy the data of the value to the cache
    store_item->value_offset = offset + store_hdr->value_section.used;
    store_item->value_len    = value->len + 1;

    memcpy(p, value->data, value->len);
    p += value->len;
    *p = '\0';

    store_hdr->value_section.used += store_item->value_len;

    return PUPA_OK;
}

#if (_PUPA_DARWIN)
static int _pupa_store_item_compare(void *p1, const void *p2, const void *arg)
#else
static int _pupa_store_item_compare(const void *p1, const void *p2, void *arg)
#endif
{
    int                        ret;
    char                      *store_hdr;
    pupa_store_item_wrapper_t *store_item_wrapper;

    ret = ((pupa_store_item_t *) p1)->key_len -
            ((pupa_store_item_t *) p2)->key_len;
    if (ret != 0) {
        return ret;
    }

    store_item_wrapper = (pupa_store_item_wrapper_t *) arg;
    store_hdr          = (char *) store_item_wrapper->ctx->store_hdr;

    if (store_item_wrapper->key_offset == 0) {
        ret = memcmp(store_hdr + ((pupa_store_item_t *) p1)->key_offset,
                     store_hdr + ((pupa_store_item_t *) p2)->key_offset,
                     ((pupa_store_item_t *) p1)->key_len);
    } else {
        ret = memcmp(store_hdr + store_item_wrapper->key_offset,
                     store_hdr + ((pupa_store_item_t *) p2)->key_offset,
                     ((pupa_store_item_t *) p1)->key_len);
    }

    return ret;
}

static int pupa_store_item_compare(const void *p1, const void *p2)
{
    pupa_store_item_wrapper_t *p_store_item_wrapper;

    p_store_item_wrapper = (pupa_store_item_wrapper_t *)
            ((char *) p1 - offsetof(pupa_store_item_wrapper_t, store_item));

    return _pupa_store_item_compare((void *) p1, p2, p_store_item_wrapper);
}

static int pupa_store_value_compaction(pupa_ctx_t *ctx, pupa_str_t *value,
                                       char **address)
{
    int                i;
    char              *p;
    size_t             used_size;
    size_t             offset;
    pupa_store_item_t *p_cache_item;

    p = PUPA_STORE_GET_MIRROR_ADDR(ctx->store_hdr,
                                   ctx->store_hdr->value_section);

    offset = PUPA_STORE_GET_MIRROR_OFFSET(ctx->store_hdr->value_section);

    used_size = 0;
    for (i = 0; i < ctx->store_hdr->item_section.used; i++) {
        p_cache_item = &ctx->store_items_mirror[i];
        memcpy(p + used_size,
               (char *) ctx->store_hdr + p_cache_item->value_offset,
               p_cache_item->value_len);
        p_cache_item->value_offset = offset + used_size;
        used_size += p_cache_item->value_len;
    }

    if ((value->len + 1 + used_size) > ctx->store_hdr->value_section.size) {
        return PUPA_OVERFLOW;
    }

    *address                           = p + used_size;
    ctx->store_hdr->value_section.used = used_size;
    ctx->store_hdr->value_section.id   =
            PUPA_STORE_GET_SEC_MIRROR_ID(ctx->store_hdr->value_section);

    return PUPA_OK;
}

static int pupa_store_key_compaction(pupa_ctx_t *ctx, pupa_str_t *key,
                                     char **address)
{
    int                i;
    char              *p;
    size_t             used_size;
    size_t             offset;
    pupa_store_item_t *p_cache_item;

    p = PUPA_STORE_GET_MIRROR_ADDR(ctx->store_hdr, ctx->store_hdr->key_section);

    offset = PUPA_STORE_GET_MIRROR_OFFSET(ctx->store_hdr->key_section);

    used_size = 0;
    for (i = 0; i < ctx->store_hdr->item_section.used; i++) {
        p_cache_item = &ctx->store_items_mirror[i];
        memcpy(p + used_size, (char *) ctx->store_hdr + p_cache_item->key_offset,
               p_cache_item->key_len);
        p_cache_item->key_offset = offset + used_size;
        used_size += p_cache_item->key_len;
    }

    if ((key->len + 1 + used_size) > ctx->store_hdr->key_section.size) {
        return PUPA_OVERFLOW;
    }

    *address                         = p;
    ctx->store_hdr->key_section.used = used_size;
    ctx->store_hdr->key_section.id   =
            PUPA_STORE_GET_SEC_MIRROR_ID(ctx->store_hdr->key_section);

    return PUPA_OK;
}

static void pupa_store_item_make_mirror(pupa_ctx_t *ctx)
{
    pupa_store_item_t *p_cache_items;

    p_cache_items = (pupa_store_item_t *) PUPA_STORE_GET_MIRROR_ADDR(
        ctx->store_hdr, ctx->store_hdr->item_section);

    if (ctx->store_hdr->item_section.used) {
        memcpy(p_cache_items, ctx->store_items,
               sizeof(pupa_store_item_t) * ctx->store_hdr->item_section.used);
    }

    ctx->store_items_mirror = p_cache_items;
}

int pupa_cache_dump(pupa_ctx_t *ctx)
{
    int                i;
    pupa_store_item_t *p_item;

    DEBUG_LOG("------- PUPA DUMP -------");

    p_item = ctx->store_items;
    for (i = 0; i < ctx->store_hdr->item_section.used; i++) {
        DEBUG_LOG("index: %d", i);
        DEBUG_LOG("key: %.*s", p_item[i].key_len,
                  (char *) ctx->store_hdr + p_item[i].key_offset);
        DEBUG_LOG("value: %.*s", p_item[i].value_len,
                  (char *) ctx->store_hdr + p_item[i].value_offset);
    }

    p_item = ctx->store_items_mirror;
    for (i = 0; i < ctx->store_hdr->item_section.used; i++) {
        DEBUG_LOG("index: %d", i);
        DEBUG_LOG("key: %.*s", p_item[i].key_len,
                  (char *) ctx->store_hdr + p_item[i].key_offset);
        DEBUG_LOG("value: %.*s", p_item[i].value_len,
                  (char *) ctx->store_hdr + p_item[i].value_offset);
    }

    return PUPA_OK;
}

int pupa_store_stats(pupa_ctx_t *ctx, pupa_str_t *stat)
{
    static char buf[1024];
    char *      p = buf;

    stat->len = sprintf(
        p,
        "{\n"
        "\t\"item\" : {\n"
        "\t\t\"size\": %zu,\n"
        "\t\t\"used\": %zu,\n"
        "\t\t\"id\": %d,\n"
        "\t\t\"section-1\": %zu,\n"
        "\t\t\"section-2\": %zu,\n"
        "\t},\n"
        "\t\"key\" : {\n"
        "\t\t\"size\": %zu,\n"
        "\t\t\"used\": %zu,\n"
        "\t\t\"id\": %d,\n"
        "\t\t\"section-1\": %zu,\n"
        "\t\t\"section-2\": %zu,\n"
        "\t},\n"
        "\t\"value\" : {\n"
        "\t\t\"size\": %zu,\n"
        "\t\t\"used\": %zu,\n"
        "\t\t\"id\": %d,\n"
        "\t\t\"section-1\": %zu,\n"
        "\t\t\"section-2\": %zu,\n"
        "\t}\n"
        "}\n",
        ctx->store_hdr->item_section.size, ctx->store_hdr->item_section.used,
        ctx->store_hdr->item_section.id,
        ctx->store_hdr->item_section.sec1_offset,
        ctx->store_hdr->item_section.sec2_offset,
        ctx->store_hdr->key_section.size, ctx->store_hdr->key_section.used,
        ctx->store_hdr->key_section.id, ctx->store_hdr->key_section.sec1_offset,
        ctx->store_hdr->key_section.sec2_offset,
        ctx->store_hdr->value_section.size, ctx->store_hdr->value_section.used,
        ctx->store_hdr->value_section.id,
        ctx->store_hdr->value_section.sec1_offset,
        ctx->store_hdr->value_section.sec2_offset);

    stat->data = buf;

    pupa_cache_dump(ctx);

    return PUPA_OK;
}

int pupa_store_fini(pupa_ctx_t *ctx)
{
    int ret;

    ret = pupa_shm_fini(ctx);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}
