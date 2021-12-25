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
    // Tree *t = tree_new();

    // tree_create(t, "/a/");
    // tree_create(t, "/b/");
    // tree_create(t, "/c/");

    // char *l = tree_list(t, "/");
    // printf("\n\n%s\n\n", l);
    // free(l);

    // tree_remove(t, "/a/");
    // tree_remove(t, "/b/");
    // tree_remove(t, "/c/");


    // tree_create(t, "/a/");
    // tree_create(t, "/b/");
    // tree_create(t, "/c/");

    // tree_move(t, "/a/", "/b/c/");

    // l = tree_list(t, "/");
    // printf("\n\n%s\n\n", l);
    // free(l);

    // tree_free(t);

    Tree *t = tree_new();
    tree_create(t, "/a/");
    tree_create(t, "/a/c/");
    tree_move(t, "/a/", "/b/");

    char *l = tree_list(t, "/");
    printf("\n\n%s\n\n", l);
    free(l);

    l = tree_list(t, "/b/");
    printf("\n\n%s\n\n", l);
    free(l);

    tree_create(t, "/a/");
    tree_create(t, "/a/c/");
    tree_create(t, "/a/d/");
    tree_create(t, "/a/g/");

    tree_move(t, "/a/c/", "/a/e/");

    l = tree_list(t, "/");
    printf("\n\n%s\n\n", l);
    free(l);

    l = tree_list(t, "/a/");
    printf("\n\n%s\n\n", l);
    free(l);

    tree_create(t, "/a/e/xxx/");

    l = tree_list(t, "/a/e/");
    printf("\n\n%s\n\n", l);
    free(l);

    tree_create(t, "/a/e/xxx/d/");
    tree_create(t, "/a/e/xxx/d/e/");
    tree_create(t, "/a/e/xxx/d/e/f/");
    tree_create(t, "/a/e/xxx/d/e/f/g/");

    tree_move(t, "/a/e/xxx/d/e/f/g/", "/a/e/xxx/d/e/f/h/");

    l = tree_list(t, "/a/e/xxx/d/e/f/");
    printf("\n\n%s\n\n", l);
    free(l);

    tree_create(t, "/x/");
    tree_create(t, "/x/y/");
    tree_create(t, "/x/y/z/");

    tree_create(t, "/j/");
    tree_create(t, "/j/k/");
    tree_create(t, "/j/k/l/");

    tree_move(t, "/x/y/z/", "/j/k/l/x/");

    l = tree_list(t, "/x/y/");
    printf("\n\n%s\n\n", l);
    free(l);

    l = tree_list(t, "/j/k/l/");
    printf("\n\n%s\n\n", l);
    free(l);
    return 0;
}