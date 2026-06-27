#ifndef REDIS_CLONE_HASHMAP_H
#define REDIS_CLONE_HASHMAP_H

#include <stdbool.h>
#include <stddef.h>

typedef void (*HashMapFreeValue)(void *value);
typedef void (*HashMapEachFn)(const char *key, void *value, void *context);

typedef struct HashEntry {
    char *key;
    void *value;
    struct HashEntry *next;
} HashEntry;

typedef struct HashMap {
    size_t bucket_count;
    size_t size;
    HashEntry **buckets;
    HashMapFreeValue free_value;
} HashMap;

bool hashmap_init(HashMap *map, size_t initial_buckets, HashMapFreeValue free_value);
void hashmap_destroy(HashMap *map);

bool hashmap_set(HashMap *map, const char *key, void *value);
void *hashmap_get(const HashMap *map, const char *key);
bool hashmap_del(HashMap *map, const char *key);
bool hashmap_exists(const HashMap *map, const char *key);
size_t hashmap_size(const HashMap *map);

char **hashmap_keys(const HashMap *map, size_t *count_out);
void hashmap_free_keys(char **keys, size_t count);

void hashmap_clear(HashMap *map);
void hashmap_foreach(const HashMap *map, HashMapEachFn fn, void *context);

#endif // REDIS_CLONE_HASHMAP_H
