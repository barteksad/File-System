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

static int N_TIMES = 100;

void* creator1(void *data)
{
    Tree *t = (Tree *) data;
    char *l;
    for(int i = 0; i < N_TIMES; i++)
    {

        tree_create(t, "/a/");
        tree_create(t, "/a/b/");
        tree_create(t, "/a/b/c/");

        l = tree_list(t, "/a/b/");
        if(l)
            free(l);

    }
        return 0;
}

void* mover1(void *data)
{
    Tree *t = (Tree *) data;
    char *l;
    for(int i = 0; i < N_TIMES; i++)
    {

        tree_move(t, "/a/b/c/", "/a/b/d/");

    }
        return 0;
}

void* destructer1(void *data)
{
    Tree *t = (Tree *) data;
    char *l;
    for(int i = 0; i < N_TIMES; i++)
    {

        tree_remove(t, "/a/b/c/");
        tree_remove(t, "/a/b/");
        tree_remove(t, "/a/");


    }
    return 0;
}

void* mixer1(void *data)
{
    Tree *t = (Tree *) data;
    char *l;
    for(int i = 0; i < N_TIMES; i++)
    {

        tree_create(t, "/x/");
        tree_create(t, "/k/");
        tree_create(t, "/k/l/");
        tree_create(t, "/k/l/m/");
        tree_move(t, "/x/", "/y/");
        tree_move(t, "/k/l/m/", "/y/c/");
        tree_remove(t, "/y/");

    }
    return 0;
}

int con_ok5(void)
{
    static int N_GROUPS = 10;

    Tree *t = tree_new();
    pthread_t threads[N_GROUPS * 4];
    pthread_attr_t attr;
    int err;
    void *retval;

    if ((err = pthread_attr_init(&attr)) != 0)
        syserr(err, "attr_init failed");
    if ((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) != 0)
        syserr(err, "attr_setdetachstate failed");

    for (size_t i = 0; i < N_GROUPS; i ++)
    {
        if ((err = pthread_create(&threads[i*4], &attr, creator1, t)) != 0)
            syserr(err, "create 1 failed");
        if ((err = pthread_create(&threads[i*4 + 1], &attr, mover1, t)) != 0)
            syserr(err, "create 1 failed");
        if ((err = pthread_create(&threads[i*4 + 2], &attr, destructer1, t)) != 0)
            syserr(err, "create 1 failed");
        if ((err = pthread_create(&threads[i*4 + 3], &attr, mixer1, t)) != 0)
            syserr(err, "create 1 failed");
    }

    for (size_t i = 0; i < N_GROUPS*4; i ++)
    {
        if ((err = pthread_join(threads[i], &retval)) != 0)
            syserr(err, "join 1 failed");
    }

    tree_free(t);

    if ((err = pthread_attr_destroy (&attr)) != 0)
        syserr (err, "cond destroy failed");

    return 0;
}