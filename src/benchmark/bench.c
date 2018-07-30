/*
 * Copyright (C) agile6v
 */


#include "pupa.h"
#include "pupa_config.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define DEFAULT_CACHE_FILE      "./pupa.store"
#define DEFAULT_KEY_COUNT       10000

static pid_t   parent_pid, child_pid;
static pid_t  *child_pids;
static int     process_num;
static int     live_process;

static void usage(const char *prog)
{
    fprintf(stderr, "About: PUPA Benchmark Tool\n\n"
                    "Usage: %s [option]\n\n"
                    "Options:\n"
                    "    -r      Specify the number of processes for read operation.\n"
                    "            The default is 30.\n",
            prog);
}



void signal_handler(int signo, siginfo_t *siginfo, void *ctx)
{
    int   i, status;
    pid_t pid;

    if (child_pid == 0) {
        if (signo == SIGCHLD) {
            //  reap the child process
            for (;;) {
                pid = waitpid(-1, &status, WNOHANG);
                if (pid == 0) {
                    return;
                }

                if (pid == -1) {
                    if (errno == EINTR) {
                        continue;
                    }
                    return;
                }

                printf("The child process %d exited!\n", pid);
                live_process--;
            }
        } else if (signo == SIGINT) {
            for (i = 0; i < (process_num + 1); i++) {
                kill(child_pids[i], SIGKILL);
            }
        }
    }
}

int init_signals()
{
    struct sigaction   sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        printf("install signal error %d.\n", errno);
        return -1 ;
    }

    if (sigaction(SIGINT, &sa, NULL) < 0) {
        printf("install signal error %d.\n", errno);
        return -1 ;
    }

    return 0;
}

static int parse_cmd(int argc, char *argv[], int *process_num)
{
    int c;
    int ret = 0;

    while ((c = getopt (argc, argv, "r:")) != -1) {
        switch (c) {
            case 'r':
                ret = atoi(optarg);
                if (ret == -1) {
                    printf("The parameter of \"r\" must be an positive integer.\n\n");
                    return ret;
                }
                *process_num = ret;
                break;
            default:
                ret = -1;
                break;
        }
    }

    return ret;
}

static int write_process()
{
    int     ret, count, index;
    char    key_buf[64];
    char    val_buf[126];
    pupa_str_t key, value;

    child_pid = getpid();

    printf("Start to write operation %d.\n", child_pid);
    
    ret = pupa_init(DEFAULT_CACHE_FILE, DEFAULT_KEY_COUNT, PUPA_OP_TYPE_RW);
    if (ret != PUPA_OK) {
        printf("Failed to initialize pupa.\n");
        return ret;
    }

    memset(val_buf, 1, sizeof(val_buf) - 1);
    value.data = val_buf;
    value.len = sizeof(val_buf) - 1;
    value.data[value.len] = '\0';

    key.data = key_buf;

    srand((int) time(0));
    count = 0;

    for (;;) {
        index = count + (int) (10000.0 * rand() / (RAND_MAX + 1.0));

        key.len = sprintf(key.data, "pupa-%d", index);

        ret = pupa_set(&key, &value);
        if (ret != PUPA_OK) {
            printf("Failed to execute pupa_set.\n");
            break;
        }

        if (count++ == 2147483647) {
            count = 0;
        }

        usleep(5);
    }

    pupa_fini();

    return 0;
}

static int read_process()
{
    int     ret, count, index;
    char    key_buf[64];
    char    val_buf[126];
    pupa_str_t key, value;

    child_pid = getpid();
    printf("Start to read operation %d !\n", child_pid);

    ret = pupa_init(DEFAULT_CACHE_FILE, DEFAULT_KEY_COUNT, PUPA_OP_TYPE_RO);
    if (ret != PUPA_OK) {
        printf("Failed to initialize pupa.\n");
        return ret;
    }

    key.data = key_buf;
    value.data = val_buf;

    srand((int) time(0));
    count = 0;

    for (;;) {
        index = count + (int) (10000.0 * rand() / (RAND_MAX + 1.0));

        key.len = sprintf(key.data, "pupa-%d", index);

        ret = pupa_get(&key, &value);
        if (ret == PUPA_NOT_FOUND) {
            continue;
        }

        if (ret != PUPA_OK) {
            printf("Failed to get %.*s.\n", key.len, key.data);
            break;
        }

        if (count++ == 2147483647) {
            count = 0;
        }

        usleep(2);
    }

    pupa_fini();
    return 0;
}

int make_process(int read_process_num)
{
    int   i, ret;
    pid_t pid;

    //  create the write operation process
    pid = fork();
    if (pid < 0) {
        printf("create the write operation process error : %d\n", errno);
        ret = -1;
    } else if (pid == 0) {
        write_process();
    } else if (pid > 0) {
        child_pids[0] = pid;

        usleep(200);

        //  create the read operation process
        for (i = 0; i < read_process_num; i++) {
            pid = fork();
            if (pid < 0) {
                printf("create the read operation process error : %d\n", errno);
                ret = -1;
            } else if (pid == 0) {
                read_process();
                break;
            } else if (pid > 0) {
                child_pids[i + 1] = pid;
            }
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int          ret;
    sigset_t     set;

    ret = parse_cmd(argc, argv, &process_num);
    if (ret == -1) {
        usage(argv[0]);
        return 0;
    }

    if (process_num == 0) {
        process_num = 20;
    }

    printf("The number of processes for read operation is %d\n", process_num);

    child_pids = (pid_t *) malloc(sizeof(pid_t) * (1 + process_num));

    parent_pid = getpid();

    init_signals();

    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGINT);
    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        printf("sigprocmask error : %d\n", errno);
    }

    sigemptyset(&set);

    ret = make_process(process_num);
    if (ret != 0) {
        return ret;
    }

    if (child_pid == 0) {
        live_process = process_num + 1;
        while (live_process) {
            sigsuspend(&set);
        }
    }

    return 0;
}