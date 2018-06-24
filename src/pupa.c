/*
 * Copyright (C) agile6v
 */

#include "pupa.h"

#define PUPA_DEFAULT_KEY_COUNT  1000

static pupa_ctx_t   pupa_ctx;


int pupa_init(char *path, int key_count, int op_type)
{
    int             ret;
    int32_t         len;
    pupa_cache_hdr_t  cache_hdr;

    if (pupa_ctx.init) {
        DEBUG_LOG("This library has been initialized.");
        return PUPA_OK;
    }

    len = pupa_cache_init(&cache_hdr, key_count);

    pupa_ctx.shm.size = len;
    pupa_ctx.shm.path = strdup(path);

    ret = pupa_shm_init(&pupa_ctx, op_type);
    if (ret == PUPA_ERROR) {
        return ret;
    }

    DEBUG_LOG("xx1");

    pupa_ctx.cache_hdr = (pupa_cache_hdr_t *) pupa_ctx.shm.data;

    DEBUG_LOG("xx2");

    if (!pupa_ctx.shm.exists) {
        memcpy((void *) pupa_ctx.cache_hdr,
               (void *) &cache_hdr,
               sizeof(pupa_cache_hdr_t));
    }

    DEBUG_LOG("xx3");

    pupa_ctx.init = 1;

    return PUPA_OK;
}


int pupa_get(pupa_str_t *key, pupa_str_t *value)
{
    int ret;

    ret = pupa_cache_get(&pupa_ctx, key, value);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}


int pupa_set(pupa_str_t *key, pupa_str_t *value)
{
    int ret;

    ret = pupa_cache_set(&pupa_ctx, key, value);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}


int pupa_del(pupa_str_t *key)
{
    int ret;

    ret = pupa_cache_del(&pupa_ctx, key);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}


int pupa_stats(pupa_str_t *stat_json)
{
    int                 ret;
    pupa_cache_stats_t  stat;

    ret = pupa_cache_stats(&pupa_ctx, &stat);
    if (ret != PUPA_OK) {
        return ret;
    }

    //  TODO:   serialize struct stats to json string

    return PUPA_OK;
}


int pupa_fini()
{
    int ret;

    if (pupa_ctx.init) {
        ret = pupa_cache_fini(&pupa_ctx);
        if (ret != PUPA_OK) {
            return ret;
        }
        pupa_ctx.init = 0;
    }

    return PUPA_OK;
}
