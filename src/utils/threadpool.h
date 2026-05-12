#ifndef UTILS_THREADPOOL_H
#define UTILS_THREADPOOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <threads.h>

// worker_id is always a real background worker thread id in
// [0, threadpool_get_num_threads(pool)).
typedef void (*ThreadPoolJobFn)(void *context, size_t start_index, size_t end_index, int worker_id);

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
    int next_job;

    ThreadPoolJobFn job_fn;
    void *job_context;
    size_t job_count;
} ThreadPool;

bool threadpool_init(ThreadPool *threadpool, int num_threads);
bool threadpool_quit(ThreadPool *threadpool);

// Synchronous. Dispatches all chunks to worker threads; the caller waits and
// never calls job_fn directly. Returns active worker jobs, capped at
// threadpool_get_num_threads(threadpool), or 0 when no work was submitted.
int threadpool_parallel_for(ThreadPool *threadpool, size_t count, ThreadPoolJobFn job_fn, void *job_context);
int threadpool_get_num_threads(const ThreadPool *threadpool);

#endif // UTILS_THREADPOOL_H
