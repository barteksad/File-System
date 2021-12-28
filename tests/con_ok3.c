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
#include <time.h>


static void* run3(void *data)
{
    Tree *t = (Tree *) data;

    unsigned int *mystate = malloc(sizeof(unsigned int));
    *mystate = time(NULL) ^ getpid() ^ pthread_self();

    FILE *in_file  = fopen("/mnt/c/Users/barte/OneDrive/Desktop/uw/PW/File System/tests/random_paths.txt", "r");
    char *path = malloc(sizeof(char) * 4096);

    int i = 1;
    while((fscanf(in_file, "%s", path)) == 1)
    {
        int x = rand_r(mystate) % 5;
        
        if(x)
        {
            tree_create(t, path);
        }
        else
        {
            tree_move(t, path, path);
            // printf("\n\n\n");
            char *l = tree_list(t, "/");
            // printf("%s\n", l);
            free(l);
            tree_remove(t, path);
        }

        i++;
    }

    free(path);

    fclose(in_file);

    free(mystate);

    return 0;
}

int con_ok3(void)
{
    static int N_THREADS = 20;

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
        if ((err = pthread_create(&threads[i], &attr, run3, t)) != 0)
            syserr(err, "create 1 failed");
    }
    // printf("%ld\n", map);

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