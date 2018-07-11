/*
 * Copyright (C) agile6v
 */

#ifndef _PUPA_STORE_H
#define _PUPA_STORE_H

#include "pupa_config.h"

#define PUPA_KEY_AVERAGE_LEN        64
#define PUPA_VALUE_AVERAGE_LEN      256

#define PUPA_STORE_SECTION_ONE      1
#define PUPA_STORE_SECTION_TWO      2


#define PUPA_STORE_GET_SEC_MIRROR_ID(section)                                  \
    ((section.id == PUPA_STORE_SECTION_ONE) ? PUPA_STORE_SECTION_TWO           \
                                            : PUPA_STORE_SECTION_ONE)

#define PUPA_STORE_GET_OFFSET(section)                                         \
    ((section.id == PUPA_STORE_SECTION_ONE) ? section.sec1_offset              \
                                            : section.sec2_offset)

#define PUPA_STORE_GET_MIRROR_OFFSET(section)                                  \
    ((section.id == PUPA_STORE_SECTION_ONE) ? section.sec2_offset              \
                                            : section.sec1_offset)

#define PUPA_STORE_GET_ADDR(store_hdr, section)                                \
    ((section.id == PUPA_STORE_SECTION_ONE)                                    \
         ? (char *)store_hdr + section.sec1_offset                             \
         : (char *)store_hdr + section.sec2_offset)

#define PUPA_STORE_GET_MIRROR_ADDR(store_hdr, section)                         \
    ((section.id == PUPA_STORE_SECTION_ONE)                                    \
         ? (char *)store_hdr + section.sec2_offset                             \
         : (char *)store_hdr + section.sec1_offset)

#define PUPA_STORE_GET_FREE_ADDR(store_hdr, section)                           \
    ((section.id == PUPA_STORE_SECTION_ONE)                                    \
         ? (char *)store_hdr + section.sec1_offset + section.used              \
         : (char *)store_hdr + section.sec2_offset + section.used)

typedef struct {
    uint8_t id;
    size_t  size;
    size_t  used;
    size_t  sec1_offset;
    size_t  sec2_offset;
} pupa_store_section_t;

typedef struct {
    size_t   key_offset;
    size_t   value_offset;

    uint32_t key_len;
    uint32_t value_len;
} pupa_store_item_t;

typedef struct {
    pupa_store_section_t  item_section;
    pupa_store_section_t  key_section;
    pupa_store_section_t  value_section;
} pupa_store_hdr_t;

struct pupa_ctx_s {
    uint8_t            init;  //  initialization switch
    pupa_shm_t         shm;
    pupa_store_hdr_t  *store_hdr;
    pupa_store_item_t *store_items;
    pupa_store_item_t *store_items_mirror;
};

typedef struct {
    pupa_store_item_t store_item;
    pupa_ctx_t       *ctx;
    int64_t           key_offset;
} pupa_store_item_wrapper_t;

int32_t pupa_store_init(pupa_store_hdr_t *store_hdr, int key_count);
int     pupa_store_fini(pupa_ctx_t *ctx);

int pupa_store_del(pupa_ctx_t *ctx, pupa_str_t *key);
int pupa_store_stats(pupa_ctx_t *ctx, pupa_str_t *stat);
int pupa_store_get(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value);
int pupa_store_set(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value);

#endif  //_PUPA_STORE_H
