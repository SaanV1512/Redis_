#ifndef REDIS_CLONE_THREADPOOL_H
#define REDIS_CLONE_THREADPOOL_H

#include <stdbool.h>
#include <stddef.h>

typedef struct ThreadPool ThreadPool;

bool threadpool_init(ThreadPool **pool, size_t thread_count);
bool threadpool_submit(ThreadPool *pool, void (*function)(void *), void *arg);
void threadpool_shutdown(ThreadPool *pool);
void threadpool_destroy(ThreadPool *pool);

#endif // REDIS_CLONE_THREADPOOL_H
