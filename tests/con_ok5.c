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

static void* creator1(void *data)
{
    for(int i = 0; i < 1; i++)
    {
        Tree *t = (Tree *) data;
        char *l;

        tree_create(t, "/a/");
        tree_create(t, "/b/");
        tree_create(t, "/a/c/");


    }
        return 0;
}

static void* mover1(void *data)
{
    for(int i = 0; i < 1; i++)
    {
        Tree *t = (Tree *) data;
        char *l;

        tree_move(t, "/a/c/", "/b/c/");

    }
        return 0;
}

static void* destructer1(void *data)
{
    for(int i = 0; i < 1; i++)
    {
        Tree *t = (Tree *) data;
        char *l;

        tree_remove(t, "/b/c/");
        tree_remove(t, "/b/");
        tree_remove(t, "/a/");

    }
    return 0;
}

int con_ok5(void)
{
    static int N_GROUPS = 1;

    Tree *t = tree_new();
    pthread_t threads[N_GROUPS * 3];
    pthread_attr_t attr;
    int err;
    void *retval;

    if ((err = pthread_attr_init(&attr)) != 0)
        syserr(err, "attr_init failed");
    if ((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) != 0)
        syserr(err, "attr_setdetachstate failed");

    for (size_t i = 0; i < N_GROUPS; i ++)
    {
        if ((err = pthread_create(&threads[i*3], &attr, creator1, t)) != 0)
            syserr(err, "create 1 failed");
        if ((err = pthread_create(&threads[i*3 + 1], &attr, mover1, t)) != 0)
            syserr(err, "create 1 failed");
        if ((err = pthread_create(&threads[i*3 + 2], &attr, destructer1, t)) != 0)
            syserr(err, "create 1 failed");
    }

    for (size_t i = 0; i < N_GROUPS*3; i ++)
    {
        if ((err = pthread_join(threads[i], &retval)) != 0)
            syserr(err, "join 1 failed");
    }

    tree_free(t);

    if ((err = pthread_attr_destroy (&attr)) != 0)
        syserr (err, "cond destroy failed");

    return 0;
}