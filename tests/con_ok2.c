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
#include <errno.h>

static void* run2(void *data)
{
    Tree *t = (Tree *) data;
    int err = 0;

    err |= tree_create(t, "/a/");
    err |= tree_create(t, "/a/b/");
    err |= tree_create(t, "/a/b/c/");
    err |= tree_remove(t, "/a/b/c/");

    err |= tree_create(t, "/a/b/c/");
    err |= tree_create(t, "/a/b/d/");
    err |= tree_create(t, "/a/b/c/f/");
    err |= tree_create(t, "/a/b/d/g/");
    
    err |= tree_create(t, "/a/b/c/f/h/");
    err |= tree_create(t, "/a/b/d/g/i/");

    err |= tree_move(t, "/a/b/c/f/", "/a/b/d/g/i/x/");

    char *l;

    l = tree_list(t, "/a/b/c/");
    // assert(strcmp(l, "") == 0);
    free(l);

    l = tree_list(t, "/a/b/d/g/i/");
    // assert(strcmp(l, "x") == 0);
    free(l);

    l = tree_list(t, "/a/b/d/g/i/x/");
    // assert(strcmp(l, "h") == 0);
    free(l);

    // assert(err == 0);


    err = tree_create(t, "/a/b/d/g/i/x/");
    // assert(err == EEXIST);

    err = tree_create(t, "/");
    // assert(err == EEXIST);

    err = tree_move(t, "/a/b/c/f/", "/a/b/d/g/i/x/");
    // assert(err == ENOENT);

    err = tree_move(t, "/a/b/c/", "/a/b/d/g/i/y/j/");
    // assert(err == ENOENT);

    err = tree_move(t, "/a/b/c/", "/a/b/d/g/i/y/");
    // assert(err == 0);

    err = tree_create(t, "/a/b/d/g/i/h/");

    err = tree_move(t, "/a/b/d/g/i/h/", "/a/b/d/g/i/x/");
    // assert(err == EEXIST);

    err = 0;
    err = tree_remove(t, "/a/b/d/g/i/");
    // assert(err == ENOTEMPTY);

    err = tree_remove(t, "/a/b/d/g/i/x/h/");
    // assert(err == 0);

    err = tree_remove(t, "/a/b/d/g/i/x/");
    // assert(err == 0);
}

int con_ok2(void)
{
    static int N_THREADS = 8;

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
    // print_map(get_tree_map(t), 0);

    tree_free(t);

    if ((err = pthread_attr_destroy (&attr)) != 0)
        syserr (err, "cond destroy failed");

    return 0;
}