#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdatomic.h>

#include "HashMap.h"
#include "Utils.h"

// We fix the number of hash buckets for simplicity.
#define N_BUCKETS 8

typedef struct MapPair MapPair;

struct MapPair
{
    char *key;
    HashMap *value;
    MapPair *next; // Next item in a single-linked list.
    ReadWrite map_pair_guard;
};

struct HashMap
{
    MapPair *buckets[N_BUCKETS]; // Linked lists of key-value pairs.
    ReadWrite *buckets_guards[N_BUCKETS];
    atomic_size_t size; // total number of entries in map. must be atomic
};

static unsigned int get_hash(const char *key);

HashMap *hmap_new()
{
    HashMap *map = malloc(sizeof(HashMap));
    if (!map)
        return NULL;
    memset(map, 0, sizeof(HashMap));

    for (int h = 0; h < N_BUCKETS; h++)
    {
        map->buckets_guards[h] = malloc(sizeof(ReadWrite));
        rw_init(map->buckets_guards[h]);
    }

    return map;
}

void hmap_free(HashMap *map)
{
    int err;

    for (int h = 0; h < N_BUCKETS; ++h)
    {
        for (MapPair *p = map->buckets[h]; p;)
        {
            MapPair *q = p;
            p = p->next;
            free(q->key);
            free(q);
        }
        if((err = rw_destroy(map->buckets_guards[h])) != 0)
            return err;
        free(map->buckets_guards[h]);
    }
    free(map);
}

Pair *hmap_find(HashMap *map, int h, const char *key, access_type a_type)
{
    Pair* p = malloc(sizeof(Pair));
    p->pair_guard = &map->buckets[h]->map_pair_guard;

    rw_action_wrapper(&map->buckets[h]->map_pair_guard, a_type);
    for (MapPair *mp = map->buckets[h]; mp; mp = mp->next)
    {
        if (strcmp(key, mp->key) == 0)
        {
            p->value = mp;
            return p;
        }
    }
    p->value = NULL;
    return p;
}

Pair *hmap_get(HashMap *map, const char *key, access_type a_type)
{
    int h = get_hash(key);
    Pair *p = hmap_find(map, h, key, a_type);
    return p;
}

int hmap_insert(HashMap *map, const char *key, void *value, bool unlock_bucket_after)
{
    int err;

    if (!value)
        return EINVAL;
    int h = get_hash(key);
    Pair *p = hmap_find(map, h, key, START_READ);
    if (p->value) {
        rw_action_wrapper(&p->pair_guard, END_READ);
        return EEXIST; // Already exists.
    }
    MapPair *new_mp = malloc(sizeof(MapPair));
    new_mp->key = strdup(key);
    new_mp->value = value;
    new_mp->next = map->buckets[h];
    if((err = rw_init(&new_mp->map_pair_guard)) != 0)
        return err;
    map->buckets[h] = new_mp;
    map->size++;
    if (unlock_bucket_after) {
        if((err = rw_action_wrapper(&p->pair_guard, END_READ)) != 0)
            return err;
    }
    return 0;
}

int hmap_remove(HashMap *map, const char *key)
{
    int err;

    int h = get_hash(key);
    MapPair **pp = &(map->buckets[h]);
    if((err = rw_action_wrapper(&map->buckets_guards[h], START_WRITE)) != 0)
        return err;

    while (*pp)
    {
        MapPair *p = *pp;
        if (strcmp(key, p->key) == 0)
        {
            // if not empty
            if (hmap_size(p->value) > 0) {
                rw_action_wrapper(&map->buckets_guards[h], END_WRITE);
                return ENOTEMPTY;
            }
            // to jest chyba jednak nie potrzebne
            // rw_action_wrapper(&p->map_pair_guard, ERASE);
            *pp = p->next;
            free(p->key);
            free(p);
            map->size--;
            rw_action_wrapper(&map->buckets_guards[h], END_WRITE);
            return 0;
        }
        pp = &(p->next);
    }
    return ENOENT;
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
    MapPair *p = it->pair;
    while (!p && it->bucket < N_BUCKETS - 1)
    {
        p = map->buckets[++it->bucket];
    }
    if (!p)
        return false;
    *key = p->key;
    *value = p->value;
    it->pair = p->next;
    return true;
}

static unsigned int get_hash(const char *key)
{
    unsigned int hash = 17;
    while (*key)
    {
        hash = (hash << 3) + hash + *key;
        ++key;
    }
    return hash % N_BUCKETS;
}
