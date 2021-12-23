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

    FILE *in_file  = fopen("/home/students/inf/b/bs429589/PW/File-System/tests/random_paths.txt", "r");
    char *path = malloc(sizeof(char) * 4096);

    int i = 1;
    while((fscanf(in_file, "%s", path)) == 1)
    {
        int x = rand_r(mystate) % 5;
        
        if(x)
        {
            // printf("create %d\n", i);
            tree_create(t, path);
        }
        else
        {
            // printf("remove %d\n", i);
            tree_remove(t, path);
        }

        // break;

        i++;
    }

    free(path);

    fclose(in_file);

    free(mystate);

    return 0;
}

int con_ok3(void)
{
    static int N_THREADS = 200;

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

    printf("\n\n\n");
    // print_map(get_tree_map(t), 0);

    // char *tmp[12];
    // tmp[0] =  "/a/";
    // tmp[1] =  "/b/";
    // tmp[2] =  "/c/";
    // tmp[3] =  "/d/";
    // tmp[4] =  "/e/";
    // tmp[5] =  "/f/";
    // tmp[6] =  "/g/";
    // tmp[7] =  "/h/";
    // tmp[8] =  "/i/";
    // tmp[9] =  "/j/";
    // tmp[10] = "/k/";
    // tmp[11] = "/l/";

    tree_free(t);

    if ((err = pthread_attr_destroy (&attr)) != 0)
        syserr (err, "cond destroy failed");

    return 0;
}