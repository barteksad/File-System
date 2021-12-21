#include "../HashMap.h"
#include "test_utils.c"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

const char *t = "aabbccddeeffgghhiijjkkllmmnnoouupprrssttuuwwxxyyzzaabbccddeeffgghhiijjkkllmmnnoouupprrssttuuwwxxyyzzaabbccddeeffgghhiijjkkllmmnnoouupprrssttuuwwxxyyzzaabbccddeeffgghhiijjkkllmmnnoouupprrssttuuwwxxyyzz";

int seq_ok2(void)
{
    Pair *p;
    HashMap *map = hmap_new();

    for (int n = 0; n < 100; n++)
    {

        for (size_t i = 0; t[i] != '\0'; i++)
        {
            try_insert(map, &t[i], hmap_new(), false);
        }

        // print_map(map);

        for (size_t i = 0; t[i] != '\0'; i++)
        {
            p = hmap_get(map, &t[i], START_READ);
            if (p == NULL)
                continue;
            rw_action_wrapper(p->bucket_guard, END_READ);
            free(p);
        }

        for (size_t i = 0; t[i] != '\0'; i++)
        {
            p = hmap_remove(map, &t[i]);
            if (p == NULL)
                continue;
            rw_action_wrapper(p->bucket_guard, END_WRITE);
            hmap_free(p->value);
            free(p);
        }
    }

    hmap_free(map);

    return 0;
}