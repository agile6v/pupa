/*
 * Copyright (C) agile6v
 */

#include "pupa_config.h"

#define PUPA_DEFAULT_KEY_COUNT 1000

static pupa_ctx_t pupa_ctx;

int pupa_init(char *path, int key_count, int op_type)
{
    int              ret;
    pupa_store_hdr_t store_hdr;

    if (pupa_ctx.init) {
        DEBUG_LOG("This library has been initialized.");
        return PUPA_ERROR;
    }

    if (op_type == PUPA_OP_TYPE_RW) {
        pupa_ctx.shm.size = pupa_store_init(&store_hdr, key_count);
    }

    pupa_ctx.shm.path = strdup(path);

    ret = pupa_shm_init(&pupa_ctx, op_type);
    if (ret == PUPA_ERROR) {
        return ret;
    }

    pupa_ctx.store_hdr = (pupa_store_hdr_t *)pupa_ctx.shm.data;

    if (!pupa_ctx.shm.exists) {
        memcpy((char *)pupa_ctx.store_hdr, (char *)&store_hdr,
               sizeof(pupa_store_hdr_t));
    }

    pupa_ctx.store_items = (pupa_store_item_t *) PUPA_STORE_GET_ADDR(
        pupa_ctx.store_hdr, pupa_ctx.store_hdr->item_section);

    pupa_ctx.store_items_mirror =
        (pupa_store_item_t *) PUPA_STORE_GET_MIRROR_ADDR(
            pupa_ctx.store_hdr, pupa_ctx.store_hdr->item_section);

    pupa_ctx.init = 1;

    //  Debug Information
    DEBUG_LOG("------- PUPA Cache Item-------");
    DEBUG_LOG("size: %zu", pupa_ctx.store_hdr->item_section.size);
    DEBUG_LOG("used: %zu", pupa_ctx.store_hdr->item_section.used);
    DEBUG_LOG("id: %d", pupa_ctx.store_hdr->item_section.id);
    DEBUG_LOG("section-1: %zu", pupa_ctx.store_hdr->item_section.sec1_offset);
    DEBUG_LOG("section-2: %zu", pupa_ctx.store_hdr->item_section.sec2_offset);

    DEBUG_LOG("------- PUPA Cache Key-------");
    DEBUG_LOG("size: %zu", pupa_ctx.store_hdr->key_section.size);
    DEBUG_LOG("used: %zu", pupa_ctx.store_hdr->key_section.used);
    DEBUG_LOG("id: %d", pupa_ctx.store_hdr->key_section.id);
    DEBUG_LOG("section-1: %zu", pupa_ctx.store_hdr->key_section.sec1_offset);
    DEBUG_LOG("section-2: %zu", pupa_ctx.store_hdr->key_section.sec2_offset);

    DEBUG_LOG("------- PUPA Cache Value-------");
    DEBUG_LOG("size: %zu", pupa_ctx.store_hdr->value_section.size);
    DEBUG_LOG("used: %zu", pupa_ctx.store_hdr->value_section.used);
    DEBUG_LOG("id: %d", pupa_ctx.store_hdr->value_section.id);
    DEBUG_LOG("section-1: %zu", pupa_ctx.store_hdr->value_section.sec1_offset);
    DEBUG_LOG("section-2: %zu", pupa_ctx.store_hdr->value_section.sec2_offset);

    return PUPA_OK;
}

int pupa_get(pupa_str_t *key, pupa_str_t *value)
{
    int ret;

    ret = pupa_store_get(&pupa_ctx, key, value);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}

int pupa_set(pupa_str_t *key, pupa_str_t *value)
{
    int ret;

    ret = pupa_store_set(&pupa_ctx, key, value);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}

int pupa_del(pupa_str_t *key)
{
    int ret;

    ret = pupa_store_del(&pupa_ctx, key);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}

int pupa_stats(pupa_str_t *stat_json)
{
    int ret;

    ret = pupa_store_stats(&pupa_ctx, stat_json);
    if (ret != PUPA_OK) {
        return ret;
    }

    return PUPA_OK;
}

int pupa_fini()
{
    int ret;

    if (pupa_ctx.init) {
        ret = pupa_store_fini(&pupa_ctx);
        if (ret != PUPA_OK) {
            return ret;
        }
        pupa_ctx.init = 0;
    }

    return PUPA_OK;
}
