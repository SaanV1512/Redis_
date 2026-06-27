#include "persistence.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

static bool write_string(FILE *file, const char *value) {
    uint32_t length = (uint32_t)(strlen(value) + 1);
    if (fwrite(&length, sizeof(length), 1, file) != 1) {
        return false;
    }
    return fwrite(value, 1, length, file) == length;
}

static char *read_string(FILE *file) {
    uint32_t length = 0;
    if (fread(&length, sizeof(length), 1, file) != 1) {
        return NULL;
    }
    if (length == 0) {
        return NULL;
    }
    char *buffer = malloc(length);
    if (!buffer) {
        return NULL;
    }
    if (fread(buffer, 1, length, file) != length) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

static void save_entry(const char *key, void *value, void *context) {
    FILE *file = context;
    if (!file || !key || !value) {
        return;
    }
    DatabaseValue *db_val = value;
    write_string(file, key);
    write_string(file, db_val->value);
    uint64_t expire_at = (uint64_t)db_val->expire_at;
    fwrite(&expire_at, sizeof(expire_at), 1, file);
}

bool persistence_save(const char *filename) {
    if (!filename) {
        return false;
    }
    FILE *file = fopen(filename, "wb");
    if (!file) {
        return false;
    }

        database_lock();
    size_t count = database_size_unlocked();
    if (fwrite(&count, sizeof(count), 1, file) != 1) {
        database_unlock();
        fclose(file);
        return false;
    }
    database_foreach_unlocked(save_entry, file);
    database_unlock();
    fclose(file);
    return true;
}

bool persistence_load(const char *filename) {
    if (!filename) {
        return false;
    }

    FILE *file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    size_t count = 0;
    if (fread(&count, sizeof(count), 1, file) != 1) {
        fclose(file);
        return false;
    }

    for (size_t i = 0; i < count; ++i) {
        char *key = read_string(file);
        char *value = read_string(file);
        uint64_t expire_at = 0;
        if (!key || !value || fread(&expire_at, sizeof(expire_at), 1, file) != 1) {
            free(key);
            free(value);
            fclose(file);
            return false;
        }
        time_t when = (time_t)expire_at;
        if (!database_set(key, value)) {
            free(key);
            free(value);
            fclose(file);
            return false;
        }
        if (when != 0) {
            long ttl = (long)(when - time(NULL));
            if (ttl > 0) {
                database_expire(key, (int)ttl);
            } else {
                database_del(key);
            }
        }
        free(key);
        free(value);
    }

    fclose(file);
    return true;
}
