#ifndef APP_THREADPOOL_H
#define APP_THREADPOOL_H

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

    bool shutdown;
    uint64_t generation;
    int jobs_remaining;
    int num_threads;

    ThreadPoolJobFn job_fn;
    void *job_context;
    int job_count;
} ThreadPool;

bool threadpool_init(ThreadPool *threadpool, int num_threads);
bool threadpool_quit(ThreadPool *threadpool);

// Synchronous. Call from one controlling thread at a time; job_context must stay valid until this returns.
void threadpool_parallel_for(ThreadPool *threadpool, int count, ThreadPoolJobFn job_fn, void *job_context);
int threadpool_get_num_threads(const ThreadPool *threadpool);

#endif // APP_THREADPOOL_H
