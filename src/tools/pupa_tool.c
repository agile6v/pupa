/*
 * Copyright (C) agile6v
 */


#include "pupa.h"
#include "pupa_config.h"

#include <stdio.h>
#include <unistd.h>

#define DEFAULT_CACHE_FILE      "./pupa.store"
#define DEFAULT_KEY_COUNT       1000


static void usage(const char *prog)
{
    fprintf(stderr, "About: PUPA Debug Tool\n\n"
        "Usage: %s [option] command\n\n"
        "Commands:\n"
        "    set     Set key to the string value.\n"
        "            For example: set key value\n"
        "    get     Get the value of the key.\n"
        "            For example: get key\n"
        "    del     Delete the specified key.\n"
        "            For example: del key\n"
        "    stat    Statistics and Information about the pupa cache.\n\n"
        "Options:\n"
        "    -f      Specify the cache file of the PUPA. If not specified, \n"
        "            pupa.store file will be used in the current directory.\n"
        "    -n      Specify the number of the key. If not specified,\n"
        "            default is 1000.\n",
        prog);
}


static int pt_stat(pupa_str_t *filename)
{
    int         ret;
    pupa_str_t  stat;

    ret = pupa_init(filename->data, 0, PUPA_OP_TYPE_RO);
    if (ret != PUPA_OK) {
        printf("Failed to initialize pupa.\n");
        return ret;
    }

    ret = pupa_stats(&stat);
    if (ret != PUPA_OK) {
        printf("Failed to perform pupa_stats.\n");
        return ret;
    }

    printf("\npupa statistics: \n\n%.*s", stat.len, stat.data);

    pupa_fini();

    return 0;
}


static int pt_set(pupa_str_t *key, pupa_str_t *value,
                  int key_count, pupa_str_t *filename)
{
    int         ret;

    ret = pupa_init(filename->data, key_count, PUPA_OP_TYPE_RW);
    if (ret != PUPA_OK) {
        printf("Failed to initialize pupa.\n");
        return ret;
    }

    ret = pupa_set(key, value);
    if (ret != PUPA_OK) {
        printf("Failed to execute pupa_set.\n");
        return ret;
    }

    printf("set successfully.\n");

    pupa_fini();

    return 0;
}


static int pt_del(pupa_str_t *key, pupa_str_t *filename)
{
    int         ret;

    ret = pupa_init(filename->data, 2, PUPA_OP_TYPE_RW);
    if (ret != PUPA_OK) {
        printf("Failed to initialize pupa.\n");
        return ret;
    }

    ret = pupa_del(key);
    if (ret != PUPA_OK) {
        printf("Failed to delete %.*s.\n", key->len, key->data);
        return ret;
    }

    printf("Delete %.*s successfully.\n", key->len, key->data);

    pupa_fini();

    return 0;
}

static int pt_get(pupa_str_t *key, pupa_str_t *filename)
{
    int         ret;
    pupa_str_t  value;

    ret = pupa_init(filename->data, 0, PUPA_OP_TYPE_RO);
    if (ret != PUPA_OK) {
        printf("Failed to initialize pupa.\n");
        return ret;
    }

    ret = pupa_get(key, &value);
    if (ret != PUPA_OK) {
        printf("Failed to get %.*s.\n", key->len, key->data);
        return ret;
    }

    printf("\nGot %.*s : %.*s\n", key->len, key->data, value.len, value.data);

    pupa_fini();

    return 0;
}


static int parse_cmd(int argc, char *argv[], char **cmd, pupa_str_t *key,
                     pupa_str_t *value, pupa_str_t *filename, int *key_count)
{
    int     c, ret = 0;
    int     key_idx, cmd_idx;
    char   *command;

    if (argc < 2) {
        return -1;
    }

    cmd_idx = 1;

    while ((c = getopt (argc, argv, "f:n:")) != -1) {
        switch (c) {
            case 'n':
                cmd_idx += 2;
                ret = atoi(optarg);
                if (ret == -1) {
                    printf("The parameter of \"n\" must be an positive integer.\n\n");
                    return ret;
                }
                *key_count = ret;
                break;
            case 'f':
                cmd_idx += 2;

                filename->data = optarg;
                filename->len = strlen(optarg);
                break;
            default:
                break;
        }
    }

    if (cmd_idx != 1 && argc < 4) {
        return -1;
    }

    key_idx = cmd_idx + 1;

    command = argv[cmd_idx];

    if (!strcmp(command, "set")) {
        if (argc - key_idx != 2) {
            return -1;
        }

        key->data = argv[key_idx++];
        key->len = strlen(key->data);

        value->data = argv[key_idx];
        value->len = strlen(value->data);

    } else if (!strcmp(command, "get") || !strcmp(command, "del") ) {
        if (argc - key_idx != 1) {
            return -1;
        }

        key->data = argv[key_idx];
        key->len = strlen(key->data);
    }

    *cmd = command;

    return 0;
}


int main(int argc, char *argv[])
{
    int          ret, key_count;
    pupa_str_t   key, value;
    char        *cmd = NULL;
    pupa_str_t   filename = pupa_string(DEFAULT_CACHE_FILE);

    key_count = DEFAULT_KEY_COUNT;

    ret = parse_cmd(argc, argv, &cmd, &key, &value, &filename, &key_count);
    if (ret != 0) {
        usage(argv[0]);
        return 0;
    }

    if (!strcmp(cmd, "set")) {
        ret = pt_set(&key, &value, key_count, &filename);
    } else if (!strcmp(cmd, "get")) {
        ret = pt_get(&key, &filename);
    } else if (!strcmp(cmd, "del")) {
        ret = pt_del(&key, &filename);
    } else if (!strcmp(cmd, "stat")) {
        ret = pt_stat(&filename);
    } else {
        usage(argv[0]);
        return 0;
    }

    if (ret != PUPA_OK) {
        return ret;
    }

    return 0;
}