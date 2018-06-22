/*
 * Copyright (C) agile6v
 */

#include "pupa_config.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int pupa_shm_init(pupa_ctx_t *ctx, int op_type)
{
    int             fd;
    int             flag;
    pupa_shm_t     *shm;
    struct stat     st;

    shm = &ctx->shm;

    if (op_type == PUPA_OP_TYPE_READ) {
        fd = open(shm->path, O_RDONLY);
        if (fd == PUPA_ERROR) {
            //  TODO:   error log
            return PUPA_ERROR;
        }

        if (fstat(fd, &st) == PUPA_ERROR) {
            //  TODO:   error log
            close(fd);
            return PUPA_ERROR;
        }

        if (st.st_size == 0) {
            //  TODO:   error log
            close(fd);
            return PUPA_ERROR;
        }

        shm->size = st.st_size;
        shm->exists = 1;

        shm->data = mmap(NULL, shm->size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (shm->data == MAP_FAILED) {
            //  TODO:   error log
            close(fd);
            return -1;
        }
    } else {
        flag = O_CREAT | O_RDWR;

        fd = open(shm->path, flag);
        if (fd == PUPA_ERROR) {
            //  TODO:   error log
            return PUPA_ERROR;
        }

        if (fstat(fd, &st) == PUPA_ERROR) {
            //  TODO:   error log
            close(fd);
            return PUPA_ERROR;
        }

        shm->exists = (st.st_size == 0) ? 0 : 1;
        shm->size = (st.st_size == 0) ? shm->size : st.st_size;

        shm->data = mmap(NULL, shm->size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        if (shm->data == MAP_FAILED) {
            //  TODO:   error log
            close(fd);
            return -1;
        }
    }

    close(fd);

    return PUPA_OK;
}


int pupa_shm_sync(pupa_ctx_t *ctx)
{
    if (msync(ctx->cache_hdr, ctx->shm.size, MS_SYNC) != 0) {
        return PUPA_ERROR;
    }

    return PUPA_OK;
}


int pupa_shm_fini(pupa_ctx_t *ctx)
{
    if (munmap(ctx->cache_hdr, ctx->shm.size) != PUPA_OK) {
        //  TODO:   error log
        return PUPA_ERROR;
    }

    return PUPA_OK;
}


