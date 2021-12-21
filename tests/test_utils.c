#ifndef TEST_UTILS
#define TEST_UTILS

#include "../HashMap.h"

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

#endif