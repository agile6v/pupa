/*
 * Copyright (C) agile6v
 */

#ifndef _PUPA_SHM_H
#define _PUPA_SHM_H

struct pupa_shm_s {
    int     fd;
    char   *path;
    void   *data;
    size_t  size;
    char    exists;
};

int pupa_shm_init(pupa_ctx_t *ctx, int op_type);
int pupa_shm_sync(pupa_ctx_t *ctx);
int pupa_shm_fini(pupa_ctx_t *ctx);

#endif  //_PUPA_SHM_H
