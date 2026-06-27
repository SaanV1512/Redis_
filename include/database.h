#ifndef REDIS_CLONE_DATABASE_H
#define REDIS_CLONE_DATABASE_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

typedef struct DatabaseValue {
    char *value;
    time_t expire_at;
} DatabaseValue;

bool database_init(const char *filename);
void database_shutdown(void);

bool database_set(const char *key, const char *value);
char *database_get(const char *key);
bool database_del(const char *key);
bool database_exists(const char *key);
size_t database_size(void);
char **database_keys(size_t *count_out);
void database_free_keys(char **keys, size_t count);

bool database_flush(void);
bool database_expire(const char *key, int seconds);
long database_ttl(const char *key);

bool database_save(void);

void database_foreach(void (*fn)(const char *key, void *value, void *context), void *context);

void database_lock(void);
void database_unlock(void);
size_t database_size_unlocked(void);
void database_foreach_unlocked(void (*fn)(const char *key, void *value, void *context), void *context);

#endif // REDIS_CLONE_DATABASE_H
