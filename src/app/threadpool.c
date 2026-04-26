#include "threadpool.h"

#include <stdlib.h>

static void threadpool_get_chunk(int count, int num_threads, int worker_id, int *start_index, int *end_index) {
    int base_chunk = count / num_threads;
    int remainder = count % num_threads;
    int extra_before = worker_id < remainder ? worker_id : remainder;

    *start_index = worker_id * base_chunk + extra_before;
    *end_index = *start_index + base_chunk + (worker_id < remainder ? 1 : 0);
}

static int threadpool_worker_thread(void *param) {
    ThreadPoolWorker *worker = (ThreadPoolWorker *)param;
    ThreadPool *threadpool = worker->threadpool;

    while(true) {
        mtx_lock(&threadpool->lock);
        while(!threadpool->shutdown && worker->last_generation == threadpool->generation) {
            cnd_wait(&threadpool->cv_work, &threadpool->lock);
        }

        if(threadpool->shutdown) {
            mtx_unlock(&threadpool->lock);
            return 0;
        }

        ThreadPoolJobFn job_fn = threadpool->job_fn;
        void *job_context = threadpool->job_context;
        int job_count = threadpool->job_count;
        int num_threads = threadpool->num_threads;
        int worker_id = worker->worker_id;

        worker->last_generation = threadpool->generation;
        mtx_unlock(&threadpool->lock);

        int start_index;
        int end_index;
        threadpool_get_chunk(job_count, num_threads, worker_id, &start_index, &end_index);
        job_fn(job_context, start_index, end_index, worker_id);

        mtx_lock(&threadpool->lock);
        threadpool->jobs_remaining--;
        if(threadpool->jobs_remaining == 0) {
            cnd_broadcast(&threadpool->cv_done);
        }
        mtx_unlock(&threadpool->lock);
    }
}

static void threadpool_cleanup(ThreadPool *threadpool) {
    free(threadpool->threads);
    free(threadpool->workers);

    threadpool->threads = NULL;
    threadpool->workers = NULL;
    threadpool->num_threads = 0;
}

bool threadpool_init(ThreadPool *threadpool, int num_threads) {
    if(!threadpool) return false;
    if(num_threads < 1) num_threads = 1;

    threadpool->threads = NULL;
    threadpool->workers = NULL;
    threadpool->shutdown = false;
    threadpool->generation = 0;
    threadpool->jobs_remaining = 0;
    threadpool->num_threads = num_threads;
    threadpool->job_fn = NULL;
    threadpool->job_context = NULL;
    threadpool->job_count = 0;

    if(mtx_init(&threadpool->lock, mtx_plain) != thrd_success) {
        return false;
    }

    if(cnd_init(&threadpool->cv_work) != thrd_success) {
        mtx_destroy(&threadpool->lock);
        return false;
    }

    if(cnd_init(&threadpool->cv_done) != thrd_success) {
        cnd_destroy(&threadpool->cv_work);
        mtx_destroy(&threadpool->lock);
        return false;
    }

    threadpool->threads = (thrd_t *)calloc((size_t)num_threads, sizeof(thrd_t));
    threadpool->workers = (ThreadPoolWorker *)calloc((size_t)num_threads, sizeof(ThreadPoolWorker));
    if(!threadpool->threads || !threadpool->workers) {
        threadpool_cleanup(threadpool);
        cnd_destroy(&threadpool->cv_done);
        cnd_destroy(&threadpool->cv_work);
        mtx_destroy(&threadpool->lock);
        return false;
    }

    int created_threads = 0;
    for(int i = 0; i < num_threads; ++i) {
        threadpool->workers[i].worker_id = i;
        threadpool->workers[i].last_generation = 0;
        threadpool->workers[i].threadpool = threadpool;

        if(thrd_create(&threadpool->threads[i], threadpool_worker_thread, &threadpool->workers[i]) != thrd_success) {
            mtx_lock(&threadpool->lock);
            threadpool->shutdown = true;
            cnd_broadcast(&threadpool->cv_work);
            mtx_unlock(&threadpool->lock);

            for(int j = 0; j < created_threads; ++j) {
                thrd_join(threadpool->threads[j], NULL);
            }

            threadpool_cleanup(threadpool);
            cnd_destroy(&threadpool->cv_done);
            cnd_destroy(&threadpool->cv_work);
            mtx_destroy(&threadpool->lock);
            return false;
        }

        created_threads++;
    }

    return true;
}

void threadpool_parallel_for(ThreadPool *threadpool, int count, ThreadPoolJobFn job_fn, void *job_context) {
    if(!threadpool || !job_fn || count <= 0) return;

    mtx_lock(&threadpool->lock);
    if(threadpool->shutdown || threadpool->num_threads <= 0) {
        mtx_unlock(&threadpool->lock);
        return;
    }

    threadpool->job_fn = job_fn;
    threadpool->job_context = job_context;
    threadpool->job_count = count;
    threadpool->jobs_remaining = threadpool->num_threads;
    threadpool->generation++;

    cnd_broadcast(&threadpool->cv_work);
    while(threadpool->jobs_remaining > 0) {
        cnd_wait(&threadpool->cv_done, &threadpool->lock);
    }

    threadpool->job_fn = NULL;
    threadpool->job_context = NULL;
    threadpool->job_count = 0;
    mtx_unlock(&threadpool->lock);
}

int threadpool_get_num_threads(const ThreadPool *threadpool) {
    if(!threadpool) return 0;
    return threadpool->num_threads;
}

bool threadpool_quit(ThreadPool *threadpool) {
    if(!threadpool) return false;

    mtx_lock(&threadpool->lock);
    threadpool->shutdown = true;
    cnd_broadcast(&threadpool->cv_work);
    mtx_unlock(&threadpool->lock);

    for(int i = 0; i < threadpool->num_threads; ++i) {
        thrd_join(threadpool->threads[i], NULL);
    }

    threadpool_cleanup(threadpool);
    cnd_destroy(&threadpool->cv_done);
    cnd_destroy(&threadpool->cv_work);
    mtx_destroy(&threadpool->lock);

    return true;
}
