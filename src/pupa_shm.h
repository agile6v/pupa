/*
 * Copyright (C) agile6v
 */

#ifndef _PUPA_SHM_H
#define _PUPA_SHM_H


typedef struct {
    int     fd;
    char   *path;
    void   *data;
    size_t  size;
    char    exists;
} pupa_shm;


int pupa_shm_init(pupa_shm *shm, int op_type);

#endif //_PUPA_SHM_H
