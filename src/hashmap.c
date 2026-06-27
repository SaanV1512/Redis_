#include "hashmap.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static size_t hash_string(const char *key) {
    uint64_t hash = 1469598103934665603ULL;
    while (*key) {
        hash ^= (unsigned char)*key++;
        hash *= 1099511628211ULL;
    }
    return (size_t)hash;
}

bool hashmap_init(HashMap *map, size_t initial_buckets, HashMapFreeValue free_value) {
    if (!map || initial_buckets == 0) {
        return false;
    }

    map->bucket_count = initial_buckets;
    map->size = 0;
    map->free_value = free_value;
    map->buckets = calloc(initial_buckets, sizeof(HashEntry *));
    return map->buckets != NULL;
}

static HashEntry *hashentry_create(const char *key, void *value) {
    HashEntry *entry = malloc(sizeof(HashEntry));
    if (!entry) {
        return NULL;
    }
    entry->key = utils_strdup(key);
    entry->value = value;
    entry->next = NULL;
    if (!entry->key) {
        free(entry);
        return NULL;
    }
    return entry;
}

void hashmap_destroy(HashMap *map) {
    if (!map || !map->buckets) {
        return;
    }

    for (size_t i = 0; i < map->bucket_count; ++i) {
        HashEntry *entry = map->buckets[i];
        while (entry) {
            HashEntry *next = entry->next;
            free(entry->key);
            if (map->free_value) {
                map->free_value(entry->value);
            }
            free(entry);
            entry = next;
        }
    }
    free(map->buckets);
    map->buckets = NULL;
    map->size = 0;
    map->bucket_count = 0;
}

bool hashmap_set(HashMap *map, const char *key, void *value) {
    if (!map || !key || !map->buckets) {
        return false;
    }

    size_t index = hash_string(key) % map->bucket_count;
    HashEntry *entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (map->free_value) {
                map->free_value(entry->value);
            }
            entry->value = value;
            return true;
        }
        entry = entry->next;
    }

    HashEntry *new_entry = hashentry_create(key, value);
    if (!new_entry) {
        return false;
    }
    new_entry->next = map->buckets[index];
    map->buckets[index] = new_entry;
    map->size++;
    return true;
}

void *hashmap_get(const HashMap *map, const char *key) {
    if (!map || !key || !map->buckets) {
        return NULL;
    }

    size_t index = hash_string(key) % map->bucket_count;
    HashEntry *entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

bool hashmap_del(HashMap *map, const char *key) {
    if (!map || !key || !map->buckets) {
        return false;
    }

    size_t index = hash_string(key) % map->bucket_count;
    HashEntry *entry = map->buckets[index];
    HashEntry *prev = NULL;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev) {
                prev->next = entry->next;
            } else {
                map->buckets[index] = entry->next;
            }
            free(entry->key);
            if (map->free_value) {
                map->free_value(entry->value);
            }
            free(entry);
            map->size--;
            return true;
        }
        prev = entry;
        entry = entry->next;
    }
    return false;
}

bool hashmap_exists(const HashMap *map, const char *key) {
    return hashmap_get(map, key) != NULL;
}

size_t hashmap_size(const HashMap *map) {
    return map ? map->size : 0;
}

char **hashmap_keys(const HashMap *map, size_t *count_out) {
    if (!map || !count_out) {
        return NULL;
    }

    size_t total = 0;
    for (size_t i = 0; i < map->bucket_count; ++i) {
        HashEntry *entry = map->buckets[i];
        while (entry) {
            total++;
            entry = entry->next;
        }
    }

    char **keys = calloc(total, sizeof(char *));
    if (!keys) {
        *count_out = 0;
        return NULL;
    }

    size_t idx = 0;
    for (size_t i = 0; i < map->bucket_count; ++i) {
        HashEntry *entry = map->buckets[i];
        while (entry) {
            keys[idx++] = utils_strdup(entry->key);
            entry = entry->next;
        }
    }
    *count_out = total;
    return keys;
}

void hashmap_free_keys(char **keys, size_t count) {
    if (!keys) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        free(keys[i]);
    }
    free(keys);
}

void hashmap_clear(HashMap *map) {
    if (!map || !map->buckets) {
        return;
    }

    for (size_t i = 0; i < map->bucket_count; ++i) {
        HashEntry *entry = map->buckets[i];
        while (entry) {
            HashEntry *next = entry->next;
            free(entry->key);
            if (map->free_value) {
                map->free_value(entry->value);
            }
            free(entry);
            entry = next;
        }
        map->buckets[i] = NULL;
    }
    map->size = 0;
}

void hashmap_foreach(const HashMap *map, HashMapEachFn fn, void *context) {
    if (!map || !fn) {
        return;
    }

    for (size_t i = 0; i < map->bucket_count; ++i) {
        HashEntry *entry = map->buckets[i];
        while (entry) {
            fn(entry->key, entry->value, context);
            entry = entry->next;
        }
    }
}
