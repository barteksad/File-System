#include "../Tree.h"
#include "../err.h"
#include "test_utils.c"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "pthread.h"
#include <assert.h>

static void* run2(void *data)
{
    Tree *t = (Tree *) data;

    tree_create(t, "/a/");
    tree_remove(t, "/a/");
    tree_create(t, "/a/");

    tree_create(t, "/b/");
    tree_remove(t, "/b/");
    tree_create(t, "/b/");

    tree_create(t, "/c/");
    tree_remove(t, "/c/");
    tree_create(t, "/c/");

    tree_create(t, "/d/");
    tree_remove(t, "/d/");
    tree_create(t, "/d/j/k");

    tree_create(t, "/e/");
    tree_remove(t, "/e/");
    tree_create(t, "/e/");

    tree_create(t, "/f/");
    tree_remove(t, "/f/");
    tree_create(t, "/f/");

    tree_create(t, "/g/");
    tree_remove(t, "/g/");
    tree_create(t, "/g/");

    tree_create(t, "/h/");
    tree_remove(t, "/h/");
    tree_create(t, "/h/");

    tree_create(t, "/i/");
    tree_remove(t, "/i/");
    tree_create(t, "/i/");

    tree_create(t, "/j/");
    tree_remove(t, "/j/");
    tree_create(t, "/j/");

    tree_create(t, "/k/");
    tree_remove(t, "/k/");
    tree_create(t, "/k/");

    tree_create(t, "/l/");
    tree_remove(t, "/l/");
    tree_create(t, "/l/");

    return 0;
}

int con_ok2(void)
{
    static int N_THREADS = 1000;

    Tree *t = tree_new();
    pthread_t threads[N_THREADS];
    pthread_attr_t attr;
    int err;
    void *retval;

    if ((err = pthread_attr_init(&attr)) != 0)
        syserr(err, "attr_init failed");
    if ((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) != 0)
        syserr(err, "attr_setdetachstate failed");

    for (size_t i = 0; i < N_THREADS; i ++)
    {
        if ((err = pthread_create(&threads[i], &attr, run2, t)) != 0)
            syserr(err, "create 1 failed");
    }
    // printf("%ld\n", map);

    for (size_t i = 0; i < N_THREADS; i ++)
    {
        if ((err = pthread_join(threads[i], &retval)) != 0)
            syserr(err, "join 1 failed");
    }

    printf("\n\n\n");
    print_map(get_tree_map(t));

    char *tmp[12];
    tmp[0] =  "/a/";
    tmp[1] =  "/b/";
    tmp[2] =  "/c/";
    tmp[3] =  "/d/";
    tmp[4] =  "/e/";
    tmp[5] =  "/f/";
    tmp[6] =  "/g/";
    tmp[7] =  "/h/";
    tmp[8] =  "/i/";
    tmp[9] =  "/j/";
    tmp[10] = "/k/";
    tmp[11] = "/l/";

    // for (size_t i = 0; i < 12; i++)
    // {
    //     tree_remove(t, tmp[i]);
    // }

    tree_free(t);

    if ((err = pthread_attr_destroy (&attr)) != 0)
        syserr (err, "cond destroy failed");

    return 0;
}