#include "../HashMap.h"
#include "../Tree.h"

#include "test_utils.c"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int seq_ok3()
{
    Tree *t = tree_new();

    tree_create(t, "/a/");
    tree_create(t, "/b/");
    tree_create(t, "/c/");

    char *l = tree_list(t, "/");
    printf("\n\n%s\n\n", l);
    free(l);

    tree_remove(t, "/a/");
    tree_remove(t, "/b/");
    tree_remove(t, "/c/");


    tree_create(t, "/a/");
    tree_create(t, "/b/");
    tree_create(t, "/c/");

    tree_move(t, "/a/", "/b/c/");

    l = tree_list(t, "/");
    printf("\n\n%s\n\n", l);
    free(l);

    tree_free(t);

    return 0;
}