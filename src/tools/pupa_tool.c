/*
 * Copyright (C) agile6v
 */


#include "pupa.h"
#include "pupa_config.h"

#include <stdio.h>

#define DEFAULT_CACHE_FILE      "./pupa.cache"


static void usage(const char *prog)
{
    fprintf(stderr, "About: PUPA Debug Tool\n\n"
        "Usage: %s [option] command\n\n"
        "Commands:\n"
        "    set     Set key to the string value.\n"
        "            For example: set key value\n"
        "    get     Get the value of the key.\n"
        "            For example: get key\n"
        "    stat    Statistics and Information about the pupa cache.\n\n"
        "Options:\n"
        "    -f      Specify the cache file of the PUPA. If not specified, \n"
        "            pupa.cache file will be used in the current directory.\n",
        prog);
}


static void pt_stat()
{
    int         ret;
    pupa_str_t  stat;

    ret = pupa_stats(&stat);
    if (ret != PUPA_OK) {
        printf("Failed to execute pupa_stats.\n");
        return;
    }

    printf("\tstat: \n\n%.*s", stat.len, stat.data);
}


static int pt_set(pupa_str_t *key, pupa_str_t *value, pupa_str_t *filename)
{
    return 0;
}


static int pt_get(pupa_str_t *key, pupa_str_t *filename)
{
    return 0;
}


int main(int argc, char *argv[])
{
    char        *command;
    int          ret;
    int          key_index;
    pupa_str_t   key, value;
    pupa_str_t   filename = pupa_string(DEFAULT_CACHE_FILE);

    if (argc < 2) {
        usage(argv[0]);
        return 0;
    }

    if (!strcmp(argv[1], "-f")) {
        filename.data = argv[2];
        filename.len = strlen(argv[2]);
        command = argv[3];
        key_index = 4;
    } else {
        command = argv[1];
        key_index = 2;
    }

    if (!strcmp(command, "set")) {
        key.data = argv[key_index++];
        key.len = strlen(key.data);

        value.data = argv[key_index];
        value.len = strlen(value.data);

        ret = pt_set(&key, &value, &filename);
        if (ret != PUPA_OK) {
            return ret;
        }
    } else if (!strcmp(command, "get") && argc >= 3) {
        key.data = argv[key_index];
        key.len = strlen(key.data);

        ret = pt_get(&key, &filename);
        if (ret != PUPA_OK) {
            return ret;
        }
    } else if (!strcmp(command, "stat")) {
        pt_stat();
    } else {
        usage(argv[0]);
        return 0;
    }

    return 0;
}