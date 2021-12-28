#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdatomic.h>

#include "HashMap.h"
#include "path_utils.h"

// We fix the number of hash buckets for simplicity.
#define N_BUCKETS 8

typedef struct MapPair MapPair;

struct MapPair
{
    char *key;
    HashMap *value;
    MapPair *next; // Next item in a single-linked list.
};

struct HashMap
{
    MapPair *buckets[N_BUCKETS]; // Linked lists of key-value pairs.
    ReadWrite *buckets_guards[N_BUCKETS];
    atomic_size_t size; // total number of entries in map.
};

HashMap *hmap_new()
{
    int err = 0;

    HashMap *map = malloc(sizeof(HashMap));
    if (!map)
        return NULL;

    memset(map, 0, sizeof(HashMap));

    for (size_t h = 0; h < N_BUCKETS; h++)
    {
        map->buckets_guards[h] = malloc(sizeof(ReadWrite));
        if (!map->buckets_guards[h] || err != 0)
        {
            hmap_free(map);
            return NULL;
        }
        err = rw_init(map->buckets_guards[h]);
    }

    errno = err;
    return map;
}

int hmap_free(HashMap *map)
{
    if (map == NULL)
        return 0;

    int err = 0;

    for (int h = 0; h < N_BUCKETS; ++h)
    {
        for (MapPair *mp = map->buckets[h]; mp;)
        {
            MapPair *q = mp;
            mp = mp->next;
            err = hmap_free(q->value);
            free(q->key);
            free(q);
        }
    }

    for (size_t h = 0; h < N_BUCKETS; h++)
    {
        err = rw_destroy(map->buckets_guards[h]);
        free(map->buckets_guards[h]);
    }

    free(map);

    return err;
}

static MapPair *hmap_find(HashMap *map, int h, const char *key)
{
    for (MapPair *mp = map->buckets[h]; mp; mp = mp->next)
    {
        if (strcmp(key, mp->key) == 0)
            return mp;
    }

    errno = EEXIST;
    return NULL;
}

Pair *hmap_get(HashMap *map, const char *key, AccessType a_type)
{
    int h = get_hash(key);

    if (rw_action_wrapper(map->buckets_guards[h], a_type) != 0)
        return NULL;

    Pair *p = malloc(sizeof(Pair));
    if(!p)
    {
        if(a_type != NONE)
            rw_action_wrapper(map->buckets_guards[h], a_type + 1);
        return NULL;
    }
    MapPair *mp = hmap_find(map, h, key);
    if (mp)
        p->value = mp->value;
    else
        p->value = NULL;
    p->bucket_guard = map->buckets_guards[h];

    return p;
}


int hmap_insert(HashMap *map, const char *key, void *value, bool has_access)
{
    int err = 0;

    if (!value)
        return false;
    int h = get_hash(key);
    if (!has_access)
    {
        if (rw_action_wrapper(map->buckets_guards[h], START_WRITE) != 0)
            return errno;
    }
    MapPair *mp = hmap_find(map, h, key);
    if (mp)
    {
        if (rw_action_wrapper(map->buckets_guards[h], END_WRITE) != 0)
            return errno;
        return EEXIST;
    }
    MapPair *new_p = malloc(sizeof(MapPair));
    if(!new_p)
        err = -1;
    if(err==0)
        new_p->key = strdup(key);
    if(!new_p->key)
        err = -1;
    if(err==0)
    {
        new_p->value = value;
        new_p->next = map->buckets[h];
        map->buckets[h] = new_p;
        map->size++;
    }
    if (rw_action_wrapper(map->buckets_guards[h], END_WRITE) != 0)
        return errno;
    return err;
}

Pair *hmap_remove(HashMap *map, const char *key, bool has_access, bool must_be_empty)
{
    int h = get_hash(key);
    if(!has_access)
    {
        if (rw_action_wrapper(map->buckets_guards[h], START_WRITE) != 0)
            return NULL;
    }
    MapPair **mpp = &(map->buckets[h]);
    while (*mpp)
    {
        MapPair *mp = *mpp;
        if (strcmp(key, mp->key) == 0)
        {
            if (must_be_empty && hmap_size(mp->value) > 0)
            {
                errno = ENOTEMPTY;
                rw_action_wrapper(map->buckets_guards[h], END_WRITE);
                return NULL;
            }

            *mpp = mp->next;

            Pair *p = malloc(sizeof(Pair));
            if (!p)
            {
                rw_action_wrapper(map->buckets_guards[h], END_WRITE);
                return NULL;
            }

            p->value = mp->value;
            p->bucket_guard = map->buckets_guards[h];
            free(mp->key);
            free(mp);
            map->size--;

            return p;
        }
        mpp = &(mp->next);
    }

    if (rw_action_wrapper(map->buckets_guards[h], END_WRITE) != 0)
        return NULL;

    errno = EEXIST;
    return NULL;
}

size_t hmap_size(HashMap *map)
{
    return map->size;
}

HashMapIterator hmap_iterator(HashMap *map)
{
    HashMapIterator it = {0, map->buckets[0]};
    return it;
}

bool hmap_next(HashMap *map, HashMapIterator *it, const char **key, void **value)
{
    MapPair *mp = it->pair;
    while (!mp && it->bucket < N_BUCKETS - 1)
    {
        mp = map->buckets[++it->bucket];
    }
    if (!mp)
        return false;
    *key = mp->key;
    *value = mp->value;
    it->pair = mp->next;
    return true;
}

unsigned int get_hash(const char *key)
{
    unsigned int hash = 17;
    while (*key)
    {
        hash = (hash << 3) + hash + *key;
        ++key;
    }
    return hash % N_BUCKETS;
}

char * map_list(HashMap *map)
{
    int err = 0;

    for (int i = 0; i < N_BUCKETS; i++)
    {
        if ((err = rw_action_wrapper(map->buckets_guards[i], START_READ)) != 0)
        {
            while (i-- >= 0)
                rw_action_wrapper(map->buckets_guards[i], END_READ);
            return NULL;
        }
    }

    char* list = make_map_contents_string(map);

    for (int i = 0; i < N_BUCKETS; i++)
    {
        if ((err = rw_action_wrapper(map->buckets_guards[i], END_READ)) != 0)
        {
            free(list);
            return NULL;
        }
    }

    return list;
}
