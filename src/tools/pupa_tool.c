/*
 * Copyright (C) agile6v
 */


#include "pupa.h"
#include "pupa_config.h"

#include <stdio.h>
#include <unistd.h>

#define DEFAULT_CACHE_FILE      "./pupa.store"
#define DEFAULT_KEY_COUNT       1000
#define DEFAULT_MAX_VALUE_VER   100


static void usage(const char *prog)
{
    fprintf(stderr, "About: PUPA Debug Tool\n\n"
        "Usage: %s [option] command\n\n"
        "Commands:\n"
        "    set     Set key to the string value.\n"
        "            For example: set key value\n"
        "    get     Get the value of the key.\n"
        "            For example: get key\n"
        "                       : get key -v 3\n"
        "    del     Delete the specified key.\n"
        "            For example: del key\n"
        "    stat    Statistics and Information about the pupa store.\n\n"
        "Options:\n"
        "    -f      Specify the store file of the PUPA. If not specified, \n"
        "            pupa.store file will be used in the current directory.\n"
        "    -v      Specify the key-value version. If not specified, \n"
        "            the latest one returned. 0 for all versions.\n"
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

static int pt_get(pupa_str_t *key, pupa_str_t *filename, int kv_ver)
{
    int         i, ret;
    pupa_str_t  values[DEFAULT_MAX_VALUE_VER];

    memset(values, 0, sizeof(values));

    ret = pupa_init(filename->data, 0, PUPA_OP_TYPE_RO);
    if (ret != PUPA_OK) {
        printf("Failed to initialize pupa.\n");
        return ret;
    }

    if (kv_ver == 0) {
        ret = pupa_get_all_ver(key, values);
        if (ret != PUPA_OK) {
            printf("Failed to get (%.*s).\n", key->len, key->data);
            return ret;
        }

        printf("\nGot %.*s :", key->len, key->data);
        for (i = 0; i < DEFAULT_MAX_VALUE_VER; i++) {
            if (values[i].len == 0 || values[i].data == NULL) {
                break;
            }
            printf("\n     v%d - %.*s ", i + 1, values[i].len, values[i].data);
        }

        printf("\n");
    } else {
        ret = pupa_get_by_ver(key, &values[0], kv_ver);
        if (ret != PUPA_OK) {
            printf("Failed to get (%.*s).\n", key->len, key->data);
            return ret;
        }

        printf("\nGot %.*s : %.*s\n", key->len, key->data, values[0].len, values[0].data);
    }

    pupa_fini();

    return 0;
}


static int parse_cmd(int argc, char *argv[], char **cmd, pupa_str_t *key,
                     pupa_str_t *value, pupa_str_t *filename, int *key_count, int *kv_ver)
{
    int     c, ret = 0;
    int     cmd_idx = 1;
    int     unparsed_arg_num, cmd_arg_num;
    char   *command = NULL;

    if (argc < 2) {
        return -1;
    }

    command = argv[cmd_idx];
    unparsed_arg_num = argc - cmd_idx - 1;

    if (!strcmp(command, "set")) {
        cmd_arg_num = 2;
        if (unparsed_arg_num < cmd_arg_num) {
            return -1;
        }

        if (argv[cmd_idx + 1][0] == '-' || argv[cmd_idx + 2][0] == '-') {
            return -1;
        }

        key->data = argv[cmd_idx + 1];
        key->len = strlen(key->data);

        value->data = argv[cmd_idx + 2];
        value->len = strlen(value->data);
    } else if (!strcmp(command, "get")) {
        cmd_arg_num = 1;
        if (unparsed_arg_num < cmd_arg_num) {
            return -1;
        }

        if (argv[cmd_idx + 1][0] == '-') {
            return -1;
        }

        key->data = argv[cmd_idx + 1];
        key->len = strlen(key->data);
    } else if (!strcmp(command, "del") ) {
        cmd_arg_num = 1;
        if (unparsed_arg_num < cmd_arg_num) {
            return -1;
        }

        if (argv[cmd_idx + 1][0] == '-') {
            return -1;
        }

        key->data = argv[cmd_idx + 1];
        key->len = strlen(key->data);
    } else if (!strcmp(command, "stat") ) {
        cmd_arg_num = 0;
    } else {
        return -1;
    }

    *cmd = command;

    optind = cmd_idx + cmd_arg_num + 1;
    while ((c = getopt(argc, argv, "f:v:n:")) != -1) {
        switch (c) {
            case 'n':
                ret = atoi(optarg);
                if (ret == -1) {
                    printf("Option \"n\" must be an positive integer.\n\n");
                    return ret;
                }
                *key_count = ret;
                break;
            case 'v':
                ret = atoi(optarg);
                if (ret == -1) {
                    printf("Option \"v\" must be an positive integer.\n\n");
                    return ret;
                }
                *kv_ver = ret;
                break;
            case 'f':
                filename->data = optarg;
                filename->len = strlen(optarg);
                break;
            default:
                break;
        }
    }

    return 0;
}


int main(int argc, char *argv[])
{
    int          ret, key_count, kv_ver;
    pupa_str_t   key, value;
    char        *cmd = NULL;
    pupa_str_t   filename = pupa_string(DEFAULT_CACHE_FILE);

    kv_ver = 1;
    key_count = DEFAULT_KEY_COUNT;

    ret = parse_cmd(argc, argv, &cmd, &key, &value, &filename, &key_count, &kv_ver);
    if (ret != 0) {
        usage(argv[0]);
        return 0;
    }

    if (!strcmp(cmd, "set")) {
        ret = pt_set(&key, &value, key_count, &filename);
    } else if (!strcmp(cmd, "get")) {
        ret = pt_get(&key, &filename, kv_ver);
    } else if (!strcmp(cmd, "del")) {
        ret = pt_del(&key, &filename);
    } else if (!strcmp(cmd, "stat")) {
        ret = pt_stat(&filename);
    } else {
        usage(argv[0]);
        return 0;
    }

    if (ret != PUPA_OK) {
        printf("Error: %d\n", ret);
        return ret;
    }

    return 0;
}
