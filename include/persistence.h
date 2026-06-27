#ifndef REDIS_CLONE_PERSISTENCE_H
#define REDIS_CLONE_PERSISTENCE_H

#include <stdbool.h>

bool persistence_save(const char *filename);
bool persistence_load(const char *filename);

#endif // REDIS_CLONE_PERSISTENCE_H
