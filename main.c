#include "HashMap.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void print_map(HashMap* map) {
    const char* key = NULL;
    void* value = NULL;
    printf("Size=%zd\n", hmap_size(map));
    HashMapIterator it = hmap_iterator(map);
    while (hmap_next(map, &it, &key, &value)) {
        printf("Key=%s Value=%p\n", key, value);
    }
    printf("\n");
}

void try_insert(HashMap* map, const char * key ,void* value, bool has_access)
{
    if(hmap_insert(map, key, value, has_access) != 0)
        hmap_free(value);
}


int main(void)
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
    p = hmap_remove(map, "a", false, true);
    rw_action_wrapper(p->bucket_guard, END_WRITE);
    hmap_free(p->value);
    free(p);

    p = hmap_get(map, "b", START_READ);
    rw_action_wrapper(p->bucket_guard, END_READ);
    free(p);
    p = hmap_remove(map, "b", false, true);
    rw_action_wrapper(p->bucket_guard, END_WRITE);
    hmap_free(p->value);
    free(p);

    p = hmap_get(map, "c", START_READ);
    rw_action_wrapper(p->bucket_guard, END_READ);
    free(p);
    p = hmap_remove(map, "c", false, true);
    rw_action_wrapper(p->bucket_guard, END_WRITE);
    hmap_free(p->value);
    free(p);
    // print_map(map);

    hmap_free(map);

    return 0;
}