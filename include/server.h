#ifndef REDIS_CLONE_SERVER_H
#define REDIS_CLONE_SERVER_H

#include <stdbool.h>

bool server_start(unsigned short port);
void server_stop(void);

#endif // REDIS_CLONE_SERVER_H
