#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "HashMap.h"
#include "Utils.h"

// We fix the number of hash buckets for simplicity.
#define N_BUCKETS 8

typedef struct MapPair MapPair;

struct MapPair {
    char* key;
    void* value;
    MapPair* next; // Next item in a single-linked list.
    ReadWrite map_pair_guard;
};

struct HashMap {
    MapPair* buckets[N_BUCKETS]; // Linked lists of key-value pairs.
    ReadWrite* buckets_guards[N_BUCKETS];
    size_t size; // total number of entries in map.
};

static unsigned int get_hash(const char* key);

HashMap* hmap_new()
{
    HashMap* map = malloc(sizeof(HashMap));
    if (!map)
        return NULL;
    memset(map, 0, sizeof(HashMap));

    return map;
}

void hmap_free(HashMap* map)
{
    for (int h = 0; h < N_BUCKETS; ++h) {
        for (MapPair* p = map->buckets[h]; p;) {
            MapPair* q = p;
            p = p->next;
            free(q->key);
            free(q);
        }
        if(map->buckets_guards[h] != NULL) {
            rw_destroy(map->buckets_guards[h]);
            free(map->buckets_guards[h]);
        }
    }
    free(map);
}

static MapPair* hmap_find(HashMap* map, int h, const char* key)
{
    for (MapPair* mp = map->buckets[h]; mp; mp = mp->next) {
        if (strcmp(key, mp->key) == 0) {
            if (map->buckets_guards[h] == NULL) {
                map->buckets_guards[h] = malloc(sizeof(ReadWrite));
                rw_init(map->buckets_guards[h]);
            }
            
            Pair p = malloc(sizeof(Pair));

            return p;
        }
    }
    return NULL;
}

void* hmap_get(HashMap* map, const char* key)
{
    int h = get_hash(key);
    MapPair* p = hmap_find(map, h, key);
    if (p)
        return p->value;
    else
        return NULL;
}

bool hmap_insert(HashMap* map, const char* key, void* value)
{
    if (!value)
        return false;
    int h = get_hash(key);
    MapPair* p = hmap_find(map, h, key);
    if (p)
        return false; // Already exists.
    MapPair* new_p = malloc(sizeof(MapPair));
    new_p->key = strdup(key);
    new_p->value = value;
    new_p->next = map->buckets[h];
    map->buckets[h] = new_p;
    map->size++;
    return true;
}

bool hmap_remove(HashMap* map, const char* key)
{
    int h = get_hash(key);
    MapPair** pp = &(map->buckets[h]);
    while (*pp) {
        MapPair* p = *pp;
        if (strcmp(key, p->key) == 0) {
            *pp = p->next;
            free(p->key);
            free(p);
            map->size--;
            return true;
        }
        pp = &(p->next);
    }
    return false;
}

size_t hmap_size(HashMap* map)
{
    return map->size;
}

HashMapIterator hmap_iterator(HashMap* map)
{
    HashMapIterator it = { 0, map->buckets[0] };
    return it;
}

bool hmap_next(HashMap* map, HashMapIterator* it, const char** key, void** value)
{
    MapPair* p = it->pair;
    while (!p && it->bucket < N_BUCKETS - 1) {
        p = map->buckets[++it->bucket];
    }
    if (!p)
        return false;
    *key = p->key;
    *value = p->value;
    it->pair = p->next;
    return true;
}

static unsigned int get_hash(const char* key)
{
    unsigned int hash = 17;
    while (*key) {
        hash = (hash << 3) + hash + *key;
        ++key;
    }
    return hash % N_BUCKETS;
}
