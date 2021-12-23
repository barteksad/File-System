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

    HashMap *map = get_tree_map(t);
    print_map(map, 0);

    tree_remove(t, "/a/");
    tree_remove(t, "/b/");
    tree_remove(t, "/c/");


    tree_free(t);

    return 0;
}