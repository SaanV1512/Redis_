#include "database.h"
#include "hashmap.h"
#include "persistence.h"
#include "utils.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

static HashMap db_map;
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
static char *db_filename = NULL;

static void free_database_value(void *value) {
    if (!value) {
        return;
    }
    DatabaseValue *db_val = value;
    free(db_val->value);
    free(db_val);
}

bool database_init(const char *filename) {
    if (!filename) {
        return false;
    }

    if (!hashmap_init(&db_map, 1024, free_database_value)) {
        return false;
    }

    db_filename = utils_strdup(filename);
    if (!db_filename) {
        hashmap_destroy(&db_map);
        return false;
    }

    return persistence_load(filename);
}

void database_shutdown(void) {
    pthread_mutex_lock(&db_mutex);
    hashmap_destroy(&db_map);
    free(db_filename);
    db_filename = NULL;
    pthread_mutex_unlock(&db_mutex);
    pthread_mutex_destroy(&db_mutex);
}

static bool is_expired(const DatabaseValue *db_val) {
    if (!db_val || db_val->expire_at == 0) {
        return false;
    }
    return time(NULL) >= db_val->expire_at;
}

static DatabaseValue *database_value_create(const char *value, time_t expire_at) {
    DatabaseValue *db_val = malloc(sizeof(DatabaseValue));
    if (!db_val) {
        return NULL;
    }
    db_val->value = utils_strdup(value);
    db_val->expire_at = expire_at;
    if (!db_val->value) {
        free(db_val);
        return NULL;
    }
    return db_val;
}

static bool database_remove_if_expired(const char *key, DatabaseValue *db_val) {
    if (!db_val) {
        return false;
    }
    if (is_expired(db_val)) {
        hashmap_del(&db_map, key);
        return true;
    }
    return false;
}

bool database_set(const char *key, const char *value) {
    if (!key || !value) {
        return false;
    }

    pthread_mutex_lock(&db_mutex);
    DatabaseValue *db_val = database_value_create(value, 0);
    if (!db_val) {
        pthread_mutex_unlock(&db_mutex);
        return false;
    }
    bool result = hashmap_set(&db_map, key, db_val);
    pthread_mutex_unlock(&db_mutex);
    return result;
}

char *database_get(const char *key) {
    if (!key) {
        return NULL;
    }

    pthread_mutex_lock(&db_mutex);
    DatabaseValue *db_val = hashmap_get(&db_map, key);
    if (!db_val || database_remove_if_expired(key, db_val)) {
        pthread_mutex_unlock(&db_mutex);
        return NULL;
    }

    char *value_copy = utils_strdup(db_val->value);
    pthread_mutex_unlock(&db_mutex);
    return value_copy;
}

bool database_del(const char *key) {
    if (!key) {
        return false;
    }
    pthread_mutex_lock(&db_mutex);
    bool result = hashmap_del(&db_map, key);
    pthread_mutex_unlock(&db_mutex);
    return result;
}

bool database_exists(const char *key) {
    if (!key) {
        return false;
    }
    pthread_mutex_lock(&db_mutex);
    DatabaseValue *db_val = hashmap_get(&db_map, key);
    if (!db_val || database_remove_if_expired(key, db_val)) {
        pthread_mutex_unlock(&db_mutex);
        return false;
    }
    pthread_mutex_unlock(&db_mutex);
    return true;
}

size_t database_size(void) {
    pthread_mutex_lock(&db_mutex);
    size_t size = hashmap_size(&db_map);
    pthread_mutex_unlock(&db_mutex);
    return size;
}

char **database_keys(size_t *count_out) {
    if (!count_out) {
        return NULL;
    }
    pthread_mutex_lock(&db_mutex);
    char **keys = hashmap_keys(&db_map, count_out);
    pthread_mutex_unlock(&db_mutex);
    return keys;
}

void database_free_keys(char **keys, size_t count) {
    hashmap_free_keys(keys, count);
}

bool database_flush(void) {
    pthread_mutex_lock(&db_mutex);
    hashmap_clear(&db_map);
    pthread_mutex_unlock(&db_mutex);
    return true;
}

bool database_expire(const char *key, int seconds) {
    if (!key || seconds < 0) {
        return false;
    }

    pthread_mutex_lock(&db_mutex);
    DatabaseValue *db_val = hashmap_get(&db_map, key);
    if (!db_val) {
        pthread_mutex_unlock(&db_mutex);
        return false;
    }
    if (is_expired(db_val)) {
        hashmap_del(&db_map, key);
        pthread_mutex_unlock(&db_mutex);
        return false;
    }
    db_val->expire_at = time(NULL) + seconds;
    pthread_mutex_unlock(&db_mutex);
    return true;
}

long database_ttl(const char *key) {
    if (!key) {
        return -2;
    }

    pthread_mutex_lock(&db_mutex);
    DatabaseValue *db_val = hashmap_get(&db_map, key);
    if (!db_val) {
        pthread_mutex_unlock(&db_mutex);
        return -2;
    }
    if (is_expired(db_val)) {
        hashmap_del(&db_map, key);
        pthread_mutex_unlock(&db_mutex);
        return -2;
    }

    if (db_val->expire_at == 0) {
        pthread_mutex_unlock(&db_mutex);
        return -1;
    }
    long ttl = (long)(db_val->expire_at - time(NULL));
    if (ttl < 0) {
        ttl = -2;
    }
    pthread_mutex_unlock(&db_mutex);
    return ttl;
}

bool database_save(void) {
    if (!db_filename) {
        return false;
    }
    pthread_mutex_lock(&db_mutex);
    bool result = persistence_save(db_filename);
    pthread_mutex_unlock(&db_mutex);
    return result;
}

void database_foreach(void (*fn)(const char *key, void *value, void *context), void *context) {
    hashmap_foreach(&db_map, fn, context);
}

void database_lock(void) {
    pthread_mutex_lock(&db_mutex);
}

void database_unlock(void) {
    pthread_mutex_unlock(&db_mutex);
}

size_t database_size_unlocked(void) {
    return hashmap_size(&db_map);
}

void database_foreach_unlocked(void (*fn)(const char *key, void *value, void *context), void *context) {
    hashmap_foreach(&db_map, fn, context);
}
