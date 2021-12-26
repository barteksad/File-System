#include "../Tree.h"
#include "../err.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "pthread.h"
#include <assert.h>
#include <errno.h>

// static int N_TIMES_2 = 1;

static void* err_run1(void *data)
{
    char *l;
    Tree *t = (Tree *) data;
    // l = tree_list(t, "/");
    // printf("before r1 : %s\n", l);
    // free(l);
    int r1 = tree_create(t, "/a/aa/");
    l = tree_list(t, "/a/");
    // printf("before r2 : %s\n", l);
    free(l);
    int r2 = tree_create(t, "/b/bb/");
        l = tree_list(t, "/b/");
    // printf("after r2 : %s\n", l);
    free(l);

    char *e = strerror(r1);
    printf("R1 : %s, %ld\n", e, r1);
    // free(e);
    e = strerror(r2);
    printf("R2 : %s, %ld\n", e, r2);

    return 0;
}

static void* err_run2(void *data)
{
    sleep(1);
    char *l;
    Tree *t = (Tree *) data;
    l = tree_list(t, "/");
    // printf("before r3 : %s\n", l);
    free(l);
    int r3 = tree_move(t, "/a/", "/b/");
    l = tree_list(t, "/");
    // printf("after r3 : %s\n", l);
    free(l);
    char *e = strerror(r3);
    printf("R3 : %s, %ld\n", e, r3);
    // free(e);
    
    return 0;
}

int err1(void)
{
    static int N_GROUPS = 1;

    Tree *t = tree_new();
    tree_create(t, "/a/");

    pthread_t threads[N_GROUPS * 2];
    pthread_attr_t attr;
    int err;
    void *retval;

    if ((err = pthread_attr_init(&attr)) != 0)
        syserr(err, "attr_init failed");
    if ((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) != 0)
        syserr(err, "attr_setdetachstate failed");

    for (size_t i = 0; i < N_GROUPS; i ++)
    {
        if ((err = pthread_create(&threads[i*2], &attr, err_run1, t)) != 0)
            syserr(err, "create 1 failed");
        if ((err = pthread_create(&threads[i*2 + 1], &attr, err_run2, t)) != 0)
            syserr(err, "create 1 failed");
    }

    for (size_t i = 0; i < N_GROUPS*2; i ++)
    {
        if ((err = pthread_join(threads[i], &retval)) != 0)
            syserr(err, "join 1 failed");
    }

    tree_free(t);

    if ((err = pthread_attr_destroy (&attr)) != 0)
        syserr (err, "cond destroy failed");

    return 0;
}