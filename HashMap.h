#ifndef HASH_MAP
#define HASH_MAP


#include <stdbool.h>
#include <sys/types.h>

#include "Utils.h"

// A structure representing a mapping from keys to values.
// Keys are C-strings (null-terminated char*), all distinct.
// Values are non-null pointers (void*, which you can cast to any other pointer type).
typedef struct HashMap HashMap;

// A structure representing return type of hash map function
typedef struct Pair
{
    HashMap *value;
    ReadWrite *bucket_guard;
} Pair;

// Create a new, empty map.
HashMap* hmap_new();

// Clear the map and free its memory. This frees the map and the keys
// copied by hmap_insert, but does not free any values.
// returns errno if pthread objects destruction faild
int hmap_free(HashMap* map);

// Get the value stored under `key`, or NULL if not present.
Pair* hmap_get(HashMap* map, const char* key, AccessType a_type);

// Insert a `value` under `key` and return true,
// or do nothing and return false if `key` already exists in the map.
// `value` must not be NULL.
// (The caller can free `key` at any time - the map internally uses a copy of it).
int hmap_insert(HashMap* map, const char* key, void* value, bool has_access);

// Remove the value under `key` and return true (the value is not free'd),
// or do nothing and return false if `key` was not present.
Pair *hmap_remove(HashMap *map, const char *key, bool has_access, bool must_be_empty);

// Return the number of elements in the map.
size_t hmap_size(HashMap* map);

typedef struct HashMapIterator HashMapIterator;

// Return an iterator to the map. See `hmap_next`.
HashMapIterator hmap_iterator(HashMap* map);

unsigned int get_hash(const char *key);

// Set `*key` and `*value` to the current element pointed by iterator and
// move the iterator to the next element.
// If there are no more elements, leaves `*key` and `*value` unchanged and
// returns false.
//
// The map cannot be modified between calls to `hmap_iterator` and `hmap_next`.
//
// Usage: ```
//     const char* key;
//     void* value;
//     HashMapIterator it = hmap_iterator(map);
//     while (hmap_next(map, &it, &key, &value))
//         foo(key, value);
// ```
bool hmap_next(HashMap* map, HashMapIterator* it, const char** key, void** value);

struct HashMapIterator {
    int bucket;
    void* pair;
};

char * map_list(HashMap *map);

#endif // define hashmap
