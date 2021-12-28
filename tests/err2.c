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


static void* err_run3(void *data)
{
    char *l;
    Tree *t = (Tree *) data;

    int r = tree_create(t, "/a/aa/");
    assert(r == 0);
    r = tree_create(t, "/a/aa/");
    assert(r == EEXIST);
    r = tree_remove(t, "/");
    assert(r == EBUSY);
    r = tree_remove(t, "/a/b/c/");
    assert(r == ENOENT);
    r = tree_remove(t, "/a/");
    assert(r == ENOTEMPTY);
    r = tree_move(t, "/a/aa/", "/b/");
    assert(r == 0);
    r = tree_move(t, "/", "/b/");
    assert(r == EBUSY);
    r = tree_create(t, "/b/c/");
    assert(r == 0);
    r = tree_create(t, "/c/");
    assert(r == 0);
    r = tree_move(t, "/b/", "/c/");
    assert(r == EEXIST);
    r = tree_move(t, "/b/c/", "/c/");
    assert(r == EEXIST);
    r = tree_create(t, "/b/c/a/");
    r = r | tree_create(t, "/b/c/d/");
    r = r | tree_create(t, "/b/c/d/e/");
    r = r | tree_create(t, "/b/c/d/e/f/");
    r = r | tree_create(t, "/b/c/d/e/f/g/");
    assert(r == 0);
    r = tree_move(t, "/b/c/d/e/f/", "/b/c/d/e/f/g/");
    assert(r == -2);
    l = tree_list(t, "/b/c/d/e/f/h/");
    assert(l == NULL);
    return 0;
}


int err2(void)
{
    static int N_GROUPS = 1;

    Tree *t = tree_new();
    tree_create(t, "/a/");

    pthread_t threads[N_GROUPS * 1];
    pthread_attr_t attr;
    int err;
    void *retval;

    if ((err = pthread_attr_init(&attr)) != 0)
        syserr(err, "attr_init failed");
    if ((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) != 0)
        syserr(err, "attr_setdetachstate failed");

    for (size_t i = 0; i < N_GROUPS; i ++)
    {
        if ((err = pthread_create(&threads[i], &attr, err_run3, t)) != 0)
            syserr(err, "create 1 failed");
    }

    for (size_t i = 0; i < N_GROUPS; i ++)
    {
        if ((err = pthread_join(threads[i], &retval)) != 0)
            syserr(err, "join 1 failed");
    }

    tree_free(t);

    if ((err = pthread_attr_destroy (&attr)) != 0)
        syserr (err, "cond destroy failed");

    return 0;
}