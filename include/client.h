#ifndef REDIS_CLONE_CLIENT_H
#define REDIS_CLONE_CLIENT_H

#include <stdbool.h>

bool client_run(const char *address, unsigned short port);

#endif // REDIS_CLONE_CLIENT_H
