/*
 * Copyright (C) agile6v
 */

#ifndef _PUPA_CONFIG_H
#define _PUPA_CONFIG_H

typedef struct pupa_str_s pupa_str_t;
typedef struct pupa_shm_s pupa_shm_t;
typedef struct pupa_ctx_s pupa_ctx_t;

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pupa_shm.h"
#include "pupa_store.h"

#define PUPA_OP_TYPE_RO     1   //  read-only
#define PUPA_OP_TYPE_RW     2   //  read-write

#define PUPA_OK                      0
#define PUPA_ERROR                  -1
#define PUPA_OVERFLOW               -2
#define PUPA_NOT_FOUND              -3
#define PUPA_NOT_FOUND_VALUE        -4
#define PUPA_EXCEED_MAXIMUM_NUM     -5

#define pupa_string(str)                                                       \
    {                                                                          \
        sizeof(str) - 1, (char *)str                                           \
    }

#define pupa_str_set(str, value)                                               \
    (str)->len  = sizeof(value) - 1;                                           \
    (str)->data = (char *)value

#ifdef PUPA_DEBUG
#define DEBUG_LOG(...)                                                         \
    do {                                                                       \
        fprintf(stderr, "%s@%d: ", __FILE__, __LINE__);                        \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
    } while (0)
#else
#define DEBUG_LOG(...)
#endif

struct pupa_str_s {
    int   len;
    char *data;
};

#endif  //_PUPA_CONFIG_H
