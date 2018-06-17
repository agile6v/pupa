/*
 * Copyright (C) agile6v
 */

#include "pupa.h"
#include "pupa_shm.h"

int pupa_shm_init(pupa_shm *shm, int op_type)
{
    int         fd;
    int         flag;
    struct stat st;

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


