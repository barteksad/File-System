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

static void* run4(void *data)
{
    Tree *t = (Tree *) data;
    char *l;

    tree_create(t, "/a/");
    tree_create(t, "/a/c/");
    tree_move(t, "/a/", "/b/");

    l = tree_list(t, "/");
    free(l);

    l = tree_list(t, "/b/");
    free(l);

    tree_create(t, "/a/");
    tree_create(t, "/a/c/");
    tree_create(t, "/a/d/");
    tree_create(t, "/a/g/");

    tree_move(t, "/a/c/", "/a/e/");

    l = tree_list(t, "/");
    free(l);

    l = tree_list(t, "/a/");
    free(l);

    tree_create(t, "/a/e/xxx/");

    l = tree_list(t, "/a/e/");
    free(l);

    tree_create(t, "/a/e/xxx/d/");
    tree_create(t, "/a/e/xxx/d/e/");
    tree_create(t, "/a/e/xxx/d/e/f/");
    tree_create(t, "/a/e/xxx/d/e/f/g/");

    tree_move(t, "/a/e/xxx/d/e/f/g/", "/a/e/xxx/d/e/f/h/");

    l = tree_list(t, "/a/e/xxx/d/e/f/");
    free(l);

    tree_create(t, "/x/");
    tree_create(t, "/x/y/");
    tree_create(t, "/x/y/z/");

    tree_create(t, "/j/");
    tree_create(t, "/j/k/");
    tree_create(t, "/j/k/l/");

    tree_move(t, "/x/y/z/", "/j/k/l/m/");

    tree_remove(t, "/j/k/l/m/");

    l = tree_list(t, "/x/y/");
    free(l);

    l = tree_list(t, "/j/k/l/");
    free(l);

    t = tree_new();

    tree_create(t, "/a/");
    tree_create(t, "/a/b/");
    tree_create(t, "/a/b/c/");
    tree_create(t, "/a/b/c/d/");
    tree_create(t, "/a/b/c/d/e/");
    tree_create(t, "/a/b/c/d/e/f/");
    tree_create(t, "/a/b/c/d/e/f/a/");
    tree_create(t, "/a/b/c/d/e/f/a/b/");

    tree_create(t, "/a/b/c/d/e/f/x/");
    tree_create(t, "/a/b/c/d/e/f/x/y/");

    tree_move(t, "/a/b/c/d/e/f/a/b/", "/a/b/c/d/e/f/x/y/z/");

    l = tree_list(t, "/a/b/c/d/e/f/a/");
    free(l);

    tree_remove(t, "/a/b/c/d/e/f/x/y/z/");

    l = tree_list(t, "/a/b/c/d/e/f/x/y/");
    free(l);

    l = tree_list(t, "/a/b/c/d/e/f/");
    free(l);

    tree_move(t, "/a/b/c/d/e/f/x/y/z/", "/a/b/c/d/e/f/x/y/y/");

    l = tree_list(t, "/a/b/c/d/e/f/x/y/");
    free(l);

    tree_remove(t, "/a/b/c/d/e/f/x/y/y/");

    tree_free(t);

    return 0;
}

int con_ok4(void)
{
    static int N_THREADS = 50;

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
        if ((err = pthread_create(&threads[i], &attr, run4, t)) != 0)
            syserr(err, "create 1 failed");
    }

    for (size_t i = 0; i < N_THREADS; i ++)
    {
        if ((err = pthread_join(threads[i], &retval)) != 0)
            syserr(err, "join 1 failed");
    }

    tree_free(t);

    if ((err = pthread_attr_destroy (&attr)) != 0)
        syserr (err, "cond destroy failed");

    return 0;
}