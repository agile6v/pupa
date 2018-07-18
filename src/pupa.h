/*
 * Copyright (C) agile6v
 */

#ifndef _PUPA_H
#define _PUPA_H

#include "pupa_config.h"

#ifdef __cplusplus
extern "C"{
#endif

int pupa_init(char *path, int key_count, int op_type);

int pupa_fini();

int pupa_get(pupa_str_t *key, pupa_str_t *value);

int pupa_set(pupa_str_t *key, pupa_str_t *value);

int pupa_del(pupa_str_t *key);

int pupa_stats(pupa_str_t *stat_json);

#ifdef __cplusplus
}
#endif

#endif  //_PUPA_H
