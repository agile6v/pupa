/*
 * Copyright (C) agile6v
 */

#ifndef _PUPA_CONFIG_H
#define _PUPA_CONFIG_H


#define PUPA_OP_TYPE_READ    1
#define PUPA_OP_TYPE_WRITE   2

#define PUPA_OK          0
#define PUPA_ERROR      -1

#define pupa_string(str)     { sizeof(str) - 1, (char *) str }

#define pupa_str_set(str, value)                                     \
    (str)->len = sizeof(value) - 1; (str)->data = (char *) value

typedef struct {
    int    len;
    char  *data;
} pupa_str_t;

#endif //_PUPA_CONFIG_H
