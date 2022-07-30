/*
 * Copyright (C) agile6v
 */

#include "pupa_store.h"

static int _pupa_store_item_compare(const void *p1, const void *p2, void *arg);
static int  pupa_store_item_compare(const void *p1, const void *p2);
static void pupa_store_item_make_snapshot(pupa_ctx_t *ctx);
static int  pupa_store_item_add(pupa_ctx_t *ctx, pupa_store_item_t *store_item,
                                pupa_str_t *key, pupa_str_t *value);
static int  pupa_store_item_replace(pupa_ctx_t *ctx,
                                    pupa_store_item_t *store_item,
                                    pupa_str_t *value);
static int  pupa_store_value_compaction(pupa_ctx_t *ctx, pupa_str_t *value,
                                        char **address);
static int  pupa_store_key_compaction(pupa_ctx_t *ctx, pupa_str_t *key,
                                      char **address);
static void pupa_sort(void *base, size_t n, size_t size, void *arg,
                      int (*cmp)(const void *_a, const void *_b, void *arg));

int32_t pupa_store_init(pupa_store_hdr_t *store_hdr, int key_count, int max_ver_num)
{
    int32_t offset;

    offset = sizeof(pupa_store_hdr_t);

    store_hdr->max_ver_num = max_ver_num;

    //  initialize store item section
    store_hdr->item_section.size        = key_count;
    store_hdr->item_section.used        = 0;
    store_hdr->item_section.id          = PUPA_STORE_SECTION_ONE;
    store_hdr->item_section.sec1_offset = offset;
    offset += key_count * sizeof(pupa_store_item_t) +
            max_ver_num * sizeof(pupa_store_item_val_t);
    store_hdr->item_section.sec2_offset = offset;
    offset += key_count * sizeof(pupa_store_item_t) +
            max_ver_num * sizeof(pupa_store_item_val_t);

    //  initialize store key section
    store_hdr->key_section.size        = PUPA_KEY_AVERAGE_LEN * key_count;
    store_hdr->key_section.used        = 0;
    store_hdr->key_section.id          = PUPA_STORE_SECTION_ONE;
    store_hdr->key_section.sec1_offset = offset;
    offset += key_count * PUPA_KEY_AVERAGE_LEN;
    store_hdr->key_section.sec2_offset = offset;
    offset += key_count * PUPA_KEY_AVERAGE_LEN;

    //  initialize store value section
    store_hdr->value_section.size        = PUPA_VALUE_AVERAGE_LEN * key_count;
    store_hdr->value_section.used        = 0;
    store_hdr->value_section.id          = PUPA_STORE_SECTION_ONE;
    store_hdr->value_section.sec1_offset = offset;
    offset += key_count * PUPA_VALUE_AVERAGE_LEN;
    store_hdr->value_section.sec2_offset = offset;
    offset += key_count * PUPA_VALUE_AVERAGE_LEN;

    return offset;
}

int pupa_store_get(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value, int version)
{
    int                        item_len;
    pupa_store_item_t         *p_store_item;
    pupa_store_item_val_t     *p_item_val;
    pupa_store_section_t      *p_item_section;
    pupa_store_item_wrapper_t  store_item_wrapper;

    if (ctx->store_hdr->max_ver_num < version) {
        return PUPA_EXCEED_MAXIMUM_NUM;
    }

    ctx->store_items = (pupa_store_item_t *) PUPA_STORE_GET_ADDR(
            ctx->store_hdr, ctx->store_hdr->item_section);

    store_item_wrapper.ctx = ctx;
    store_item_wrapper.key_offset =
            (int64_t) (key->data - (char *) ctx->store_hdr);
    store_item_wrapper.store_item.key_len = key->len + 1;

    p_item_section = &ctx->store_hdr->item_section;

    item_len = sizeof(pupa_store_item_t) +
            ctx->store_hdr->max_ver_num * sizeof(pupa_store_item_val_t);

    p_store_item = bsearch(&store_item_wrapper.store_item, ctx->store_items,
                           p_item_section->used, item_len,
                           pupa_store_item_compare);
    if (p_store_item == NULL) {
        DEBUG_LOG("Not found key: %.*s", key->len, key->data);
        return PUPA_NOT_FOUND;
    }

    if (p_store_item->key_len == 0) {
        DEBUG_LOG("The length of the key (%.*s) is invalid.", key->len,
                  key->data);
        return PUPA_ERROR;
    }

    if (p_store_item->val_cnt < version) {
        DEBUG_LOG("The version (%d) of the key (%.*s) doesn't exist.", version, key->len,
                  key->data);
        return PUPA_NOT_FOUND_VALUE;
    }

    p_item_val = (pupa_store_item_val_t *) ((char *) p_store_item + sizeof(pupa_store_item_t));
    p_item_val = p_item_val + ctx->store_hdr->max_ver_num - version;

    value->data = (char *) ctx->store_hdr + p_item_val->value_offset;
    value->len  = p_item_val->value_len;

    return PUPA_OK;
}

int pupa_store_get_values(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *values)
{
    int                        i, item_len;
    pupa_store_item_t         *p_store_item;
    pupa_store_item_val_t     *p_item_val;
    pupa_store_section_t      *p_item_section;
    pupa_store_item_wrapper_t  store_item_wrapper;

    ctx->store_items = (pupa_store_item_t *) PUPA_STORE_GET_ADDR(
            ctx->store_hdr, ctx->store_hdr->item_section);

    store_item_wrapper.ctx = ctx;
    store_item_wrapper.key_offset =
            (int64_t) (key->data - (char *) ctx->store_hdr);
    store_item_wrapper.store_item.key_len = key->len + 1;

    p_item_section = &ctx->store_hdr->item_section;

    item_len = sizeof(pupa_store_item_t) +
               ctx->store_hdr->max_ver_num * sizeof(pupa_store_item_val_t);

    p_store_item = bsearch(&store_item_wrapper.store_item, ctx->store_items,
                           p_item_section->used, item_len,
                           pupa_store_item_compare);
    if (p_store_item == NULL) {
        DEBUG_LOG("Not found key: %.*s", key->len, key->data);
        return PUPA_NOT_FOUND;
    }

    if (p_store_item->key_len == 0) {
        DEBUG_LOG("The length of the key (%.*s) is invalid.", key->len,
                  key->data);
        return PUPA_ERROR;
    }

    p_item_val = (pupa_store_item_val_t *) ((char *) p_store_item + sizeof(pupa_store_item_t));

    DEBUG_LOG("The number of values is %d for (%.*s) ", p_store_item->val_cnt, key->len, key->data);

    for (i = ctx->store_hdr->max_ver_num - 1;
        (ctx->store_hdr->max_ver_num - p_store_item->val_cnt) <= i; i--)
    {
        values[ctx->store_hdr->max_ver_num - i - 1].data = (char *) ctx->store_hdr + p_item_val[i].value_offset;
        values[ctx->store_hdr->max_ver_num - i - 1].len = p_item_val[i].value_len;
    }

    return PUPA_OK;
}

int pupa_store_set(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value)
{
    int                       ret;
    int                       item_len;
    pupa_store_item_t        *p_store_item;
    pupa_store_section_t     *p_item_section;
    pupa_store_item_wrapper_t store_item_wrapper;

    //  generate the snapshot of the store items
    pupa_store_item_make_snapshot(ctx);

    p_item_section = &ctx->store_hdr->item_section;

    store_item_wrapper.ctx = ctx;
    store_item_wrapper.key_offset =
            (int64_t) (key->data - (char *) ctx->store_hdr);
    store_item_wrapper.store_item.key_len = key->len + 1;

    item_len = sizeof(pupa_store_item_t) +
               ctx->store_hdr->max_ver_num * sizeof(pupa_store_item_val_t);

    p_store_item = bsearch(&store_item_wrapper.store_item,
                           ctx->store_items_snapshot, p_item_section->used,
                           item_len, pupa_store_item_compare);
    if (p_store_item == NULL) {
        DEBUG_LOG("Add key : %.*s - %.*s", key->len, key->data, value->len,
                  value->data);
        p_store_item = (pupa_store_item_t *) ((char *) ctx->store_items_snapshot + p_item_section->used * item_len);
        ret = pupa_store_item_add(ctx, p_store_item, key, value);
        //  reset the pointer for the following procedure
        p_store_item = NULL;
    } else {
        DEBUG_LOG("Replace key(%.*s) with (%.*s)", key->len, key->data,
                  value->len, value->data);
        ret = pupa_store_item_replace(ctx, p_store_item, value);
    }

    if (ret != PUPA_OK) {
        return ret;
    }

    // Switch the store item section if this key doesn't exist
    if (p_store_item == NULL) {
        p_item_section->used++;
        store_item_wrapper.key_offset = 0;
        pupa_sort((void *) ctx->store_items_snapshot, ctx->store_hdr->item_section.used,
                  item_len, &store_item_wrapper,
                  _pupa_store_item_compare);
        ctx->store_items = ctx->store_items_snapshot;
    }

    p_item_section->id =
            PUPA_STORE_GET_SEC_SNAPSHOT_ID(ctx->store_hdr->item_section);

    ret = pupa_shm_sync(ctx);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}

int pupa_store_del(pupa_ctx_t *ctx, pupa_str_t *key)
{
    int                       ret;
    int                       item_len;
    pupa_store_item_t        *p_store_item;
    pupa_store_section_t     *p_item_section;
    pupa_store_item_wrapper_t store_item_wrapper;

    //  generate the snapshot of the store items
    pupa_store_item_make_snapshot(ctx);

    p_item_section = &ctx->store_hdr->item_section;

    store_item_wrapper.ctx = ctx;
    store_item_wrapper.key_offset =
            (int64_t) (key->data - (char *) ctx->store_hdr);
    store_item_wrapper.store_item.key_len = key->len + 1;

    item_len = sizeof(pupa_store_item_t) +
               ctx->store_hdr->max_ver_num * sizeof(pupa_store_item_val_t);

    p_store_item = bsearch(&store_item_wrapper.store_item,
                           ctx->store_items_snapshot, p_item_section->used,
                           item_len, pupa_store_item_compare);
    if (p_store_item == NULL) {
        DEBUG_LOG("Not found key: %.*s", key->len, key->data);
        return PUPA_NOT_FOUND;
    }

    if ((p_store_item - ctx->store_items_snapshot + 1) < p_item_section->used) {
        memcpy(p_store_item, p_store_item + 1,
               item_len * (p_item_section->used -
                       (p_store_item - ctx->store_items_snapshot) + 1));
    }

    p_item_section->id =
            PUPA_STORE_GET_SEC_SNAPSHOT_ID(ctx->store_hdr->item_section);
    p_item_section->used--;

    ret = pupa_shm_sync(ctx);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}

static int pupa_store_item_add(pupa_ctx_t *ctx, pupa_store_item_t *store_item,
                               pupa_str_t *key, pupa_str_t *value)
{
    int                      ret;
    size_t                   offset;
    char                    *p = NULL;
    pupa_store_hdr_t        *store_hdr = ctx->store_hdr;
    pupa_store_item_val_t   *p_item_val =
            (pupa_store_item_val_t *) ((char *) store_item + sizeof(pupa_store_item_t));

    if (store_hdr->item_section.used == store_hdr->item_section.size) {
        return PUPA_OVERFLOW;
    }

    if ((store_hdr->key_section.used + (key->len + 1)) >
        store_hdr->key_section.size)
    {
        DEBUG_LOG("Start to compact the key section.");

        offset = PUPA_STORE_GET_SNAPSHOT_OFFSET(store_hdr->key_section);

        ret = pupa_store_key_compaction(ctx, key, &p);
        if (ret != PUPA_OK) {
            return ret;
        }
    } else {
        p = PUPA_STORE_GET_FREE_ADDR(store_hdr, store_hdr->key_section);
        offset = PUPA_STORE_GET_OFFSET(store_hdr->key_section);
    }

    //  copy the data of the key to the store
    store_item->key_len    = key->len + 1;
    store_item->key_offset = offset + store_hdr->key_section.used;

    memcpy(p, key->data, key->len);
    p += key->len;
    *p = '\0';
    store_hdr->key_section.used += store_item->key_len;

    //  copy the data of the value to the store
    p = NULL;

    if ((store_hdr->value_section.used + (value->len + 1)) >
        store_hdr->value_section.size)
    {
        DEBUG_LOG("Start to compact the value section.");

        offset = PUPA_STORE_GET_SNAPSHOT_OFFSET(store_hdr->value_section);

        ret = pupa_store_value_compaction(ctx, value, &p);
        if (ret != PUPA_OK) {
            return ret;
        }
    } else {
        p = PUPA_STORE_GET_FREE_ADDR(store_hdr, store_hdr->value_section);
        offset = PUPA_STORE_GET_OFFSET(store_hdr->value_section);
    }

    memcpy(p, value->data, value->len);
    p += value->len;
    *p = '\0';

    p_item_val = p_item_val + store_hdr->max_ver_num - 1;
    p_item_val->value_offset = offset + store_hdr->value_section.used;
    p_item_val->value_len    = value->len + 1;

    store_item->val_cnt = 1;

    store_hdr->value_section.used += p_item_val->value_len;

    return PUPA_OK;
}

static int pupa_store_item_replace(pupa_ctx_t        *ctx,
                                   pupa_store_item_t *store_item,
                                   pupa_str_t        *value)
{
    size_t                   offset;
    int                      ret;
    char                    *p         = NULL;
    pupa_store_hdr_t        *store_hdr = ctx->store_hdr;
    pupa_store_item_val_t   *p_item_val =
            (pupa_store_item_val_t *) ((char *) store_item + sizeof(pupa_store_item_t));

    if ((store_hdr->value_section.used + (value->len + 1)) >
        store_hdr->value_section.size)
    {
        DEBUG_LOG("Start to compact the value section.");

        offset = PUPA_STORE_GET_SNAPSHOT_OFFSET(store_hdr->value_section);

        ret = pupa_store_value_compaction(ctx, value, &p);
        if (ret != PUPA_OK) {
            return ret;
        }
    } else {
        p = PUPA_STORE_GET_FREE_ADDR(store_hdr, store_hdr->value_section);
        offset = PUPA_STORE_GET_OFFSET(store_hdr->value_section);
    }

    memcpy(p, value->data, value->len);
    p += value->len;
    *p = '\0';

    if (store_hdr->max_ver_num == 1) {
        // bypass
    } else if (store_item->val_cnt >= store_hdr->max_ver_num) {
        // evict the old value (LRU)
        memcpy(p_item_val, p_item_val + 1,
                (store_item->val_cnt - 1) * sizeof(pupa_store_item_val_t));
        p_item_val = p_item_val + (store_item->val_cnt - 1);
    } else {
        memcpy(p_item_val + (store_hdr->max_ver_num - store_item->val_cnt - 1),
                p_item_val + (store_hdr->max_ver_num - store_item->val_cnt),
                store_item->val_cnt * sizeof(pupa_store_item_val_t));

        p_item_val = p_item_val + (store_hdr->max_ver_num - 1);
        store_item->val_cnt++;
    }

    p_item_val->value_offset = offset + store_hdr->value_section.used;
    p_item_val->value_len    = value->len + 1;

    store_hdr->value_section.used += p_item_val->value_len;

    return PUPA_OK;
}

static int _pupa_store_item_compare(const void *p1, const void *p2, void *arg)
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

    // p1 is the target value, P2 is each value in the array.
    p_store_item_wrapper = (pupa_store_item_wrapper_t *)
            ((char *) p1 - offsetof(pupa_store_item_wrapper_t, store_item));

    return _pupa_store_item_compare((void *) p1, p2, p_store_item_wrapper);
}

static int pupa_store_value_compaction(pupa_ctx_t *ctx, pupa_str_t *value,
                                       char **address)
{
    int                i, j;
    char              *p;
    size_t             used_size;
    size_t             offset;
    pupa_store_item_t *p_store_item;
    pupa_store_item_val_t *p_item_val;

    p = PUPA_STORE_GET_SNAPSHOT_ADDR(ctx->store_hdr,
                                   ctx->store_hdr->value_section);

    offset = PUPA_STORE_GET_SNAPSHOT_OFFSET(ctx->store_hdr->value_section);

    used_size = 0;
    for (i = 0; i < ctx->store_hdr->item_section.used; i++) {
        p_store_item = &ctx->store_items_snapshot[i];

        for (j = 0; j < p_store_item->val_cnt; j++) {
            p_item_val = (pupa_store_item_val_t *) (p_store_item + sizeof(pupa_store_item_t) + j * sizeof(pupa_store_item_val_t));

            memcpy(p + used_size,
                   (char *) ctx->store_hdr + p_item_val->value_offset,
                   p_item_val->value_len);
            p_item_val->value_offset = offset + used_size;
            used_size += p_item_val->value_len;
        }
    }

    if ((value->len + 1 + used_size) > ctx->store_hdr->value_section.size) {
        return PUPA_OVERFLOW;
    }

    *address                           = p + used_size;
    ctx->store_hdr->value_section.used = used_size;
    ctx->store_hdr->value_section.id   =
            PUPA_STORE_GET_SEC_SNAPSHOT_ID(ctx->store_hdr->value_section);

    return PUPA_OK;
}

static int pupa_store_key_compaction(pupa_ctx_t *ctx, pupa_str_t *key,
                                     char **address)
{
    int                i;
    char              *p;
    size_t             used_size;
    size_t             offset;
    pupa_store_item_t *p_store_item;

    p = PUPA_STORE_GET_SNAPSHOT_ADDR(ctx->store_hdr, ctx->store_hdr->key_section);

    offset = PUPA_STORE_GET_SNAPSHOT_OFFSET(ctx->store_hdr->key_section);

    used_size = 0;
    for (i = 0; i < ctx->store_hdr->item_section.used; i++) {
        p_store_item = &ctx->store_items_snapshot[i];
        memcpy(p + used_size, (char *) ctx->store_hdr + p_store_item->key_offset,
               p_store_item->key_len);
        p_store_item->key_offset = offset + used_size;
        used_size += p_store_item->key_len;
    }

    if ((key->len + 1 + used_size) > ctx->store_hdr->key_section.size) {
        return PUPA_OVERFLOW;
    }

    *address                         = p;
    ctx->store_hdr->key_section.used = used_size;
    ctx->store_hdr->key_section.id   =
            PUPA_STORE_GET_SEC_SNAPSHOT_ID(ctx->store_hdr->key_section);

    return PUPA_OK;
}

static void pupa_store_item_make_snapshot(pupa_ctx_t *ctx)
{
    int                item_len;
    pupa_store_item_t *p_store_items;

    p_store_items = (pupa_store_item_t *) PUPA_STORE_GET_SNAPSHOT_ADDR(
        ctx->store_hdr, ctx->store_hdr->item_section);

    item_len = sizeof(pupa_store_item_t) +
               ctx->store_hdr->max_ver_num * sizeof(pupa_store_item_val_t);

    if (ctx->store_hdr->item_section.used) {
        memcpy(p_store_items, ctx->store_items,
               item_len * ctx->store_hdr->item_section.used);
    }

    ctx->store_items_snapshot = p_store_items;
}

int pupa_store_dump(pupa_ctx_t *ctx)
{
    int                i, j, item_len;
    pupa_store_item_t *p_item = NULL;
    pupa_store_item_val_t *p_item_value = NULL;

    item_len = sizeof(pupa_store_item_t) +
               ctx->store_hdr->max_ver_num * sizeof(pupa_store_item_val_t);

    DEBUG_LOG("------- PUPA DUMP -------");
    (void) p_item;
    p_item = ctx->store_items;
    for (i = 0; i < ctx->store_hdr->item_section.used; i++) {
        p_item = (pupa_store_item_t *) ((char *) p_item + i * item_len);
        p_item_value = (pupa_store_item_val_t *) ((char *) p_item + sizeof(pupa_store_item_t));

        DEBUG_LOG("index: %d, key: %.*s", i,
                  p_item->key_len, (char *) ctx->store_hdr + p_item->key_offset);

        for (j = 0; j < p_item->val_cnt; j++) {
            DEBUG_LOG("           value[%d]: %.*s", j, p_item_value->value_len,
                    (char *) ctx->store_hdr + p_item_value->value_offset);
            p_item_value++;
        }
    }

    DEBUG_LOG("--------------");

    p_item = ctx->store_items_snapshot;
    for (i = 0; i < ctx->store_hdr->item_section.used; i++) {
        p_item = (pupa_store_item_t *) ((char *) p_item + i * item_len);
        p_item_value = (pupa_store_item_val_t *) ((char *) p_item + sizeof(pupa_store_item_t));

        DEBUG_LOG("index: %d, key: %.*s", i,
                  p_item->key_len, (char *) ctx->store_hdr + p_item->key_offset);

        for (j = 0; j < p_item->val_cnt; j++) {
            DEBUG_LOG("           value[%d]: %.*s", j, p_item_value->value_len,
                      (char *) ctx->store_hdr + p_item_value->value_offset);
            p_item_value++;
        }
    }

    return PUPA_OK;
}

int pupa_store_stats(pupa_ctx_t *ctx, pupa_str_t *stat)
{
    static char buf[1024];
    char       *p = buf;

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

    pupa_store_dump(ctx);

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

static int pupa_bsearch(void *elt, void *arg, void *base,
                        size_t size, size_t low, size_t high,
                        int (*cmp)(const void *_a, const void *_b, void *arg))
{
    int ret, mid;

    if (high <= low) {
        ret = cmp(elt, base + low * size, arg);
        if (ret > 0) {
            return low + 1;
        }
        return low;
    }

    mid = (low + high) / 2;

    ret = cmp(elt, base + mid * size, arg);
    if (ret == 0 ) {
        return mid + 1;
    } else if (ret > 0) {
        return pupa_bsearch(elt, arg, base, size, mid + 1, high, cmp);
    } else {
        return pupa_bsearch(elt, arg, base, size, low, mid - 1, cmp);
    }
}

// pupa_sort is implemented as the binary insertation sorting.
// In our scenario, using the specified binary insertation sorting algorithm
// performance is more better.
static void pupa_sort(void *base, size_t n, size_t size, void *arg,
                      int (*cmp)(const void *_a, const void *_b, void *arg))
{
    char *p;
    int   pos, count;

    count = n - 1;

    p = (char *) malloc(size);
    if (p == NULL) {
        return;
    }

    // take the last element to compare to the previous one
    memcpy(p, base + count * size , size);

    pos = pupa_bsearch(p, arg, base, size, 0, count, cmp);
    if (pos < count) {
        memmove(base + (pos + 1) * size, base + pos * size, (count - pos) * size);
        memcpy(base + pos * size, p, size);
    }

    free(p);
}