#ifndef UTILS_THREADPOOL_H
#define UTILS_THREADPOOL_H

#include <stdbool.h>
#include <stdint.h>
#include <threads.h>

typedef void (*ThreadPoolJobFn)(void *context, int start_index, int end_index, int worker_id);

typedef struct ThreadPoolWorker {
    int worker_id;
    uint64_t last_generation;
    struct ThreadPool *threadpool;
} ThreadPoolWorker;

typedef struct ThreadPool {
    thrd_t *threads;
    ThreadPoolWorker *workers;

    mtx_t lock;
    cnd_t cv_work;
    cnd_t cv_done;

    bool initialized;
    bool shutdown;
    uint64_t generation;
    int jobs_remaining;
    int num_threads;
    int active_jobs;

    ThreadPoolJobFn job_fn;
    void *job_context;
    int job_count;
} ThreadPool;

bool threadpool_init(ThreadPool *threadpool, int num_threads);
bool threadpool_quit(ThreadPool *threadpool);

// Synchronous. The controlling thread waits idle; it does not process a chunk.
int threadpool_parallel_for(ThreadPool *threadpool, int count, ThreadPoolJobFn job_fn, void *job_context);
int threadpool_get_num_threads(const ThreadPool *threadpool);

#endif // UTILS_THREADPOOL_H
