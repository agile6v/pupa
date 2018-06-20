/*
 * Copyright (C) agile6v
 */

#include "pupa.h"
#include "pupa_shm.h"
#include "pupa_cache.h"

static pupa_ctx   pupa_ctx;

int pupa_init(char *path, int key_count, int op_type)
{
    int             ret;
    int32_t         len;
    pupa_cache_hdr  cache_hdr = {0};

    len = pupa_cache_init(&cache_hdr, key_count);

    pupa_ctx.shm.size = len;
    pupa_ctx.shm.path = strdup(path);

    ret = pupa_shm_init(&pupa_ctx.shm, op_type);
    if (ret == PUPA_ERROR) {
        return ret;
    }

    pupa_ctx.cache_hdr = (pupa_cache_hdr *) pupa_ctx.shm.data;

    if (!pupa_ctx.shm.exists) {
        memcpy((void *) pupa_ctx.cache_hdr,
               (void *) &cache_hdr,
               sizeof(pupa_cache_hdr));
    }

    return PUPA_OK;
}

int pupa_get(pupa_str_t *key, pupa_str_t *value)
{
    int ret;

    ret = pupa_cache_get(&pupa_ctx, key);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}

int pupa_set(pupa_str_t *key, pupa_str_t *value)
{
    pupa_cache_set(key, value);

    return PUPA_OK;
}
