/*
 * Copyright (C) agile6v
 */

#ifndef _PUPA_CONFIG_H
#define _PUPA_CONFIG_H


typedef struct pupa_str_s   pupa_str_t;
typedef struct pupa_shm_s   pupa_shm_t;
typedef struct pupa_ctx_s   pupa_ctx_t;

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>

#include "pupa_shm.h"
#include "pupa_cache.h"

#define PUPA_OP_TYPE_READ    1
#define PUPA_OP_TYPE_WRITE   2

#define PUPA_OK          0
#define PUPA_ERROR      -1
#define PUPA_OVERFLOW   -2
#define PUPA_NOT_FOUND  -3

#define pupa_string(str)     { sizeof(str) - 1, (char *) str }

#define pupa_str_set(str, value)                                     \
    (str)->len = sizeof(value) - 1; (str)->data = (char *) value

struct pupa_str_s {
    int    len;
    char  *data;
};


#endif //_PUPA_CONFIG_H
