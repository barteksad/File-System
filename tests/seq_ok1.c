#include "../HashMap.h"
#include "test_utils.c"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>




int seq_ok1(void)
{
    Pair* p;

    HashMap* map = hmap_new();
    try_insert(map, "a", hmap_new(), false);
    try_insert(map, "a", hmap_new(), false);
    try_insert(map, "b", hmap_new(), false);
    try_insert(map, "c", hmap_new(), false);

    p = hmap_get(map, "a", START_READ);
    rw_action_wrapper(p->bucket_guard, END_READ);
    free(p);
    p = hmap_remove(map, "a");
    rw_action_wrapper(p->bucket_guard, END_WRITE);
    hmap_free(p->value);
    free(p);

    p = hmap_get(map, "b", START_READ);
    rw_action_wrapper(p->bucket_guard, END_READ);
    free(p);
    p = hmap_remove(map, "b");
    rw_action_wrapper(p->bucket_guard, END_WRITE);
    hmap_free(p->value);
    free(p);

    p = hmap_get(map, "c", START_READ);
    rw_action_wrapper(p->bucket_guard, END_READ);
    free(p);
    p = hmap_remove(map, "c");
    rw_action_wrapper(p->bucket_guard, END_WRITE);
    hmap_free(p->value);
    free(p);
    // print_map(map);

    hmap_free(map);

    return 0;
}