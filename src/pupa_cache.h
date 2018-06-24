/*
 * Copyright (C) agile6v
 */

#ifndef _PUPA_CACHE_H
#define _PUPA_CACHE_H

#include "pupa_config.h"

#define PUPA_KEY_AVERAGE_LEN        64
#define PUPA_VALUE_AVERAGE_LEN      256

#define PUPA_CACHE_SECTION_ONE      1
#define PUPA_CACHE_SECTION_TWO      2


#define PUPA_CACHE_GET_KEY_OFFSET(key_section) \
    ((key_section.id == PUPA_CACHE_SECTION_ONE) ? \
        key_section.sec1_offset : key_section.sec2_offset)


#define PUPA_CACHE_GET_KEY_MIRROR_OFFSET(key_section) \
    ((key_section.id == PUPA_CACHE_SECTION_ONE) ? \
        key_section.sec2_offset : key_section.sec1_offset)


#define PUPA_CACHE_GET_ADDR(cache_hdr, section) \
    ((section.id == PUPA_CACHE_SECTION_ONE) ? \
        (char *) cache_hdr + section.sec1_offset : \
        (char *) cache_hdr + section.sec2_offset)


#define PUPA_CACHE_GET_MIRROR_ADDR(cache_hdr, section) \
    ((section.id == PUPA_CACHE_SECTION_ONE) ? \
        (char *) cache_hdr + section.sec2_offset : \
        (char *) cache_hdr + section.sec1_offset)


#define PUPA_CACHE_GET_FREE_ADDR(cache_hdr, section) \
    ((section.id == PUPA_CACHE_SECTION_ONE) ? \
        (char *) cache_hdr + section.sec1_offset + section.used : \
        (char *) cache_hdr + section.sec2_offset + section.used)


typedef struct {
    uint8_t     id;
    size_t      size;
    size_t      used;
    int32_t     sec1_offset;
    int32_t     sec2_offset;
} pupa_cache_section_t;


typedef struct {
    int32_t    key_offset;
    int32_t    value_offset;

    uint16_t   key_len;
    uint16_t   value_len;
} pupa_cache_item_t;


typedef struct {
    pupa_cache_section_t  item_section;
    pupa_cache_section_t  key_section;
    pupa_cache_section_t  value_section;
} pupa_cache_hdr_t;


struct pupa_ctx_s {
    int                 init;  //  initialization switch
    pupa_shm_t          shm;
    pupa_cache_hdr_t   *cache_hdr;
    pupa_cache_item_t  *cache_items;
    pupa_cache_item_t  *cache_items_mirror;
};


typedef struct {
    pupa_cache_item_t  cache_item;
    pupa_ctx_t      *ctx;
    int32_t          key_section_offset;
} pupa_cache_item_wrapper_t;


typedef struct {

} pupa_cache_stats_t;


int32_t pupa_cache_init(pupa_cache_hdr_t *cache_hdr, int key_count);
int pupa_cache_fini(pupa_ctx_t *ctx);

int pupa_cache_del(pupa_ctx_t *ctx, pupa_str_t *key);
int pupa_cache_stats(pupa_ctx_t *ctx, pupa_cache_stats_t *stat);
int pupa_cache_get(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value);
int pupa_cache_set(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value);

#endif //_PUPA_CACHE_H
