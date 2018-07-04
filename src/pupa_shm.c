/*
 * Copyright (C) agile6v
 */

#include "pupa_config.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int pupa_shm_init(pupa_ctx_t *ctx, int op_type)
{
    int         fd;
    int         flag;
    pupa_shm_t *shm;
    struct stat st;

    shm = &ctx->shm;

    if (op_type == PUPA_OP_TYPE_R) {
        fd = open(shm->path, O_RDONLY);
        if (fd == PUPA_ERROR) {
            DEBUG_LOG("Failed to open %s, errno: %d", shm->path, errno);
            return PUPA_ERROR;
        }

        if (fstat(fd, &st) == PUPA_ERROR) {
            DEBUG_LOG("Failed to fstat %s, errno: %d", shm->path, errno);
            close(fd);
            return PUPA_ERROR;
        }

        if (st.st_size == 0) {
            DEBUG_LOG("File %s is empty.", shm->path);
            close(fd);
            return PUPA_ERROR;
        }

        shm->size   = st.st_size;
        shm->exists = 1;

        shm->data = mmap(NULL, shm->size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (shm->data == MAP_FAILED) {
            DEBUG_LOG("Failed to mmap(%s), size: %ld, errno: %d", shm->path,
                      shm->size, errno);
            close(fd);
            return PUPA_ERROR;
        }
    } else {
        flag = O_CREAT | O_RDWR;

        fd = open(shm->path, flag, 0666);
        if (fd == PUPA_ERROR) {
            DEBUG_LOG("Failed to open %s, errno: %d", shm->path, errno);
            return PUPA_ERROR;
        }

        if (fstat(fd, &st) == PUPA_ERROR) {
            DEBUG_LOG("Failed to fstat %s, errno: %d", shm->path, errno);
            close(fd);
            return PUPA_ERROR;
        }

        shm->exists = (st.st_size == 0) ? 0 : 1;
        shm->size   = (st.st_size == 0) ? shm->size : st.st_size;

        if (!shm->exists) {
            if (ftruncate(fd, shm->size) == PUPA_ERROR) {
                DEBUG_LOG("Failed to ftruncate %s, size: %ld, errno: %d",
                          shm->path, shm->size, errno);
                return PUPA_ERROR;
            }
        }

        shm->data =
            mmap(NULL, shm->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (shm->data == MAP_FAILED) {
            DEBUG_LOG("Failed to mmap(%s), size: %ld, errno: %d", shm->path,
                      shm->size, errno);
            close(fd);
            return PUPA_ERROR;
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
