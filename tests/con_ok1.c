#include "../HashMap.h"
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



static int N_THREADS = 1000;


void* run1(void *data)
{
    Pair *p;
    HashMap *map = (HashMap *)data;

    try_insert(map, "a", hmap_new(), false);

    p = hmap_remove(map, "a");
    if (p != NULL)
    {
        rw_action_wrapper(p->bucket_guard, END_WRITE);
        hmap_free(p->value);
        free(p);
    }

    try_insert(map, "b", hmap_new(), false);
    try_insert(map, "c", hmap_new(), false);
    try_insert(map, "d", hmap_new(), false);
    try_insert(map, "e", hmap_new(), false);
    try_insert(map, "f", hmap_new(), false);
    try_insert(map, "g", hmap_new(), false);
    try_insert(map, "h", hmap_new(), false);
    try_insert(map, "i", hmap_new(), false);
    try_insert(map, "j", hmap_new(), false);
    try_insert(map, "k", hmap_new(), false);
    try_insert(map, "l", hmap_new(), false);

    p = hmap_remove(map, "l");
    if (p != NULL)
    {
        rw_action_wrapper(p->bucket_guard, END_WRITE);
        hmap_free(p->value);
        free(p);
    }

    return 0;
}

int con_ok1(void)
{
    Pair *p;
    HashMap *map = hmap_new();
    // printf("%ld\n", map);
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
        if ((err = pthread_create(&threads[i], &attr, run1, map)) != 0)
            syserr(err, "create 1 failed");
    }
    // printf("%ld\n", map);

    for (size_t i = 0; i < N_THREADS; i ++)
    {
        if ((err = pthread_join(threads[i], &retval)) != 0)
            syserr(err, "join 1 failed");
    }
    // printf("%ld\n", map);

    assert(hmap_size(map) == 10);

    char *tmp[12];
    tmp[0] = "a";
    tmp[1] = "b";
    tmp[2] = "c";
    tmp[3] = "d";
    tmp[4] = "e";
    tmp[5] = "f";
    tmp[6] = "g";
    tmp[7] = "h";
    tmp[8] = "i";
    tmp[9] = "j";
    tmp[10] = "k";
    tmp[11] = "l";

    print_map(map);

    for (size_t i = 0; i < 12; i++)
    {
        p = hmap_remove(map, tmp[i]);
        if (p == NULL)
            continue;
        rw_action_wrapper(p->bucket_guard, END_WRITE);
        hmap_free(p->value);
        free(p);
    }
    printf("%ld\n", hmap_size(map));

    assert(hmap_size(map) == 0);

    // printf("%ld\n", map);

    hmap_free(map);

    if ((err = pthread_attr_destroy (&attr)) != 0)
        syserr (err, "cond destroy failed");

    return 0;
}