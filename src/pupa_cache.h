/*
 * Copyright (C) agile6v
 */

#ifndef _PUPA_CTX_H
#define _PUPA_CTX_H

#include "pupa_shm.h"

#define PUPA_KEY_AVERAGE_LEN        64
#define PUPA_VALUE_AVERAGE_LEN      256

#define PUPA_CACHE_SECTION_ONE      1
#define PUPA_CACHE_SECTION_TWO      2

typedef struct {
    uint8_t     id;
    size_t      size;
    size_t      used;
    int32_t     sec1_offset;
    int32_t     sec2_offset;
} pupa_cache_section;


typedef struct {
    int32_t    key_offset;
    int32_t    value_offset;

    uint16_t   key_len;
    uint16_t   value_len;
} pupa_cache_item;


typedef struct {
    pupa_cache_item  cache_item;
    pupa_ctx        *ctx;
    int32_t          key_section_offset;
} pupa_cache_item_wrapper;


typedef struct {
    pupa_cache_section  item_section;
    pupa_cache_section  key_section;
    pupa_cache_section  value_section;
    pupa_cache_item    *cache_items;
} pupa_cache_hdr;


typedef struct {
    pupa_shm         shm;
    pupa_cache_hdr  *cache_hdr;
} pupa_ctx;


int32_t pupa_cache_init(pupa_cache_hdr *cache_hdr, int key_count);
int pupa_cache_set(pupa_ctx *ctx, pupa_str_t *key, pupa_str_t *value);

#endif //_PUPA_CTX_H
