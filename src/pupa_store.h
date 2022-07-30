/*
 * Copyright (C) agile6v
 */

#ifndef _PUPA_STORE_H
#define _PUPA_STORE_H

#include "pupa_config.h"

#ifdef __cplusplus
extern "C"{
#endif

#define PUPA_KEY_AVERAGE_LEN        64
#define PUPA_VALUE_AVERAGE_LEN      256

#define PUPA_STORE_SECTION_ONE      1
#define PUPA_STORE_SECTION_TWO      2


#define PUPA_STORE_GET_SEC_SNAPSHOT_ID(section)                                \
    ((section.id == PUPA_STORE_SECTION_ONE) ? PUPA_STORE_SECTION_TWO           \
                                            : PUPA_STORE_SECTION_ONE)

#define PUPA_STORE_GET_OFFSET(section)                                         \
    ((section.id == PUPA_STORE_SECTION_ONE) ? section.sec1_offset              \
                                            : section.sec2_offset)

#define PUPA_STORE_GET_SNAPSHOT_OFFSET(section)                                \
    ((section.id == PUPA_STORE_SECTION_ONE) ? section.sec2_offset              \
                                            : section.sec1_offset)

#define PUPA_STORE_GET_ADDR(store_hdr, section)                                \
    ((section.id == PUPA_STORE_SECTION_ONE)                                    \
         ? (char *)store_hdr + section.sec1_offset                             \
         : (char *)store_hdr + section.sec2_offset)

#define PUPA_STORE_GET_SNAPSHOT_ADDR(store_hdr, section)                       \
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
    size_t   value_offset;
    uint32_t value_len;
} pupa_store_item_val_t;

typedef struct {
    size_t   key_offset;
    uint32_t key_len;
    int      val_cnt;
} pupa_store_item_t;

typedef struct {
    int                   max_ver_num;      // the maximum number of history value
    pupa_store_section_t  item_section;
    pupa_store_section_t  key_section;
    pupa_store_section_t  value_section;
} pupa_store_hdr_t;

struct pupa_ctx_s {
    // initialization switch
    uint8_t            init;
    pupa_shm_t         shm;
    pupa_store_hdr_t  *store_hdr;
    pupa_store_item_t *store_items;
    // All write operations first modify the snapshot area.
    pupa_store_item_t *store_items_snapshot;
};

typedef struct {
    pupa_store_item_t store_item;
    pupa_ctx_t       *ctx;
    int64_t           key_offset;
} pupa_store_item_wrapper_t;

int32_t pupa_store_init(pupa_store_hdr_t *store_hdr, int key_count, int max_ver_num);
int     pupa_store_fini(pupa_ctx_t *ctx);

int pupa_store_del(pupa_ctx_t *ctx, pupa_str_t *key);
int pupa_store_stats(pupa_ctx_t *ctx, pupa_str_t *stat);
int pupa_store_get(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value, int version);
int pupa_store_get_values(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *values);
int pupa_store_set(pupa_ctx_t *ctx, pupa_str_t *key, pupa_str_t *value);

#ifdef __cplusplus
}
#endif

#endif  //_PUPA_STORE_H
