#include "threadpool.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct ThreadPoolTask {
    void (*function)(void *);
    void *arg;
    struct ThreadPoolTask *next;
} ThreadPoolTask;

struct ThreadPool {
    pthread_t *threads;
    size_t thread_count;
    ThreadPoolTask *tasks_head;
    ThreadPoolTask *tasks_tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool running;
};

static void *threadpool_worker(void *arg) {
    ThreadPool *pool = arg;
    while (true) {
        pthread_mutex_lock(&pool->mutex);
        while (pool->running && !pool->tasks_head) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        if (!pool->running && !pool->tasks_head) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        ThreadPoolTask *task = pool->tasks_head;
        pool->tasks_head = task->next;
        if (!pool->tasks_head) {
            pool->tasks_tail = NULL;
        }
        pthread_mutex_unlock(&pool->mutex);

        task->function(task->arg);
        free(task);
    }
    return NULL;
}

bool threadpool_init(ThreadPool **pool_out, size_t thread_count) {
    if (!pool_out || thread_count == 0) {
        return false;
    }

    ThreadPool *pool = malloc(sizeof(ThreadPool));
    if (!pool) {
        return false;
    }

    pool->threads = malloc(thread_count * sizeof(pthread_t));
    if (!pool->threads) {
        free(pool);
        return false;
    }
    pool->tasks_head = NULL;
    pool->tasks_tail = NULL;
    pool->running = true;
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);

    pool->thread_count = thread_count;
    for (size_t i = 0; i < thread_count; ++i) {
        if (pthread_create(&pool->threads[i], NULL, threadpool_worker, pool) != 0) {
            pool->running = false;
            threadpool_shutdown(pool);
            free(pool);
            return false;
        }
    }
    *pool_out = pool;
    return true;
}

bool threadpool_submit(ThreadPool *pool, void (*function)(void *), void *arg) {
    if (!pool || !function) {
        return false;
    }

    ThreadPoolTask *task = malloc(sizeof(ThreadPoolTask));
    if (!task) {
        return false;
    }
    task->function = function;
    task->arg = arg;
    task->next = NULL;

    pthread_mutex_lock(&pool->mutex);
    if (!pool->running) {
        pthread_mutex_unlock(&pool->mutex);
        free(task);
        return false;
    }
    if (pool->tasks_tail) {
        pool->tasks_tail->next = task;
        pool->tasks_tail = task;
    } else {
        pool->tasks_head = task;
        pool->tasks_tail = task;
    }
    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
    return true;
}

void threadpool_shutdown(ThreadPool *pool) {
    if (!pool) {
        return;
    }
    pthread_mutex_lock(&pool->mutex);
    pool->running = false;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
    for (size_t i = 0; i < pool->thread_count; ++i) {
        pthread_join(pool->threads[i], NULL);
    }
}

void threadpool_destroy(ThreadPool *pool) {
    if (!pool) {
        return;
    }
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
    free(pool->threads);
    free(pool);
}
