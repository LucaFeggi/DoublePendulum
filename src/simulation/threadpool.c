#include "threadpool.h"

#include "../config.h"

#include <stdlib.h>
#include <math.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__) || defined(__unix__) || defined(__posix__)
#include <unistd.h>
#endif

static int worker_thread(void *param) {
    WorkerData *data = (WorkerData *)param;
    ThreadPool *threadpool = data->threadpool;

    while(true) {
        mtx_lock(&threadpool->lock);
        while(!threadpool->work_ready && !threadpool->shutdown) {
            cnd_wait(&threadpool->cv_work, &threadpool->lock);
        }

        if(threadpool->shutdown) {
            mtx_unlock(&threadpool->lock);
            return 0;
        }

        mtx_unlock(&threadpool->lock);

        /* Use pre-calculated indices */
        int start = data->start_index;
        int end = data->end_index;

        /* Perform the work and store the result directly in the WorkerData struct */
        data->local_max_ang_vel = 0.0;
        for(int i = start; i < end; ++i) {
            pendulum_update(&threadpool->pendulums[i]);
            double v0 = fabs(threadpool->pendulums[i].rod[0].ang_vel);
            double v1 = fabs(threadpool->pendulums[i].rod[1].ang_vel);
            if(v0 > data->local_max_ang_vel) data->local_max_ang_vel = v0;
            if(v1 > data->local_max_ang_vel) data->local_max_ang_vel = v1;
        }

        mtx_lock(&threadpool->lock);
        threadpool->jobs_remaining--;
        if(threadpool->jobs_remaining == 0) {
            threadpool->work_ready = false;
            cnd_broadcast(&threadpool->cv_done);
        }
        mtx_unlock(&threadpool->lock);
    }
}

/* Portable CPU count: Windows and Linux (you said you don't need macOS) */
static inline int get_num_threads() {
#if defined(_WIN32)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int n = (int)sysinfo.dwNumberOfProcessors;
    return (n > 1 ? n - 1 : 1);

#elif defined(__linux__) || defined(__unix__) || defined(__posix__)
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    if(n < 1) n = 1;
    return (n > 1 ? (int)(n - 1) : 1);

#else
    return 1; 	// returning 1 thread if neither Windows or Linux detected
#endif
}

bool threadpool_init(ThreadPool *threadpool) {
    if(!threadpool) return false;

    threadpool->num_threads = get_num_threads();
    threadpool->threads = calloc(threadpool->num_threads, sizeof(thrd_t));
    threadpool->worker_data = calloc(threadpool->num_threads, sizeof(WorkerData));

    if(!threadpool->threads || !threadpool->worker_data) {
        free(threadpool->threads);
        free(threadpool->worker_data);
        return false;
    }

    if(mtx_init(&threadpool->lock, mtx_plain) != thrd_success) {
        free(threadpool->threads);
        free(threadpool->worker_data);
        return false;
    }
    cnd_init(&threadpool->cv_work);
    cnd_init(&threadpool->cv_done);

    threadpool->work_ready = false;
    threadpool->shutdown = false;
    threadpool->jobs_remaining = 0;

    int chunk_size = TOTAL_PENDULUMS / threadpool->num_threads;
    int remainder = TOTAL_PENDULUMS % threadpool->num_threads;
    int current_start = 0;

    for(int i = 0; i < threadpool->num_threads; ++i) {
        threadpool->worker_data[i].threadpool = threadpool;
        threadpool->worker_data[i].thread_id = i;
        threadpool->worker_data[i].local_max_ang_vel = 0.0;

        int chunk = chunk_size + (i < remainder ? 1 : 0);
        threadpool->worker_data[i].start_index = current_start;
        threadpool->worker_data[i].end_index = current_start + chunk;
        current_start += chunk;

        if(thrd_create(&threadpool->threads[i], worker_thread, &threadpool->worker_data[i]) != thrd_success) {
            /* cleanup any already created threads */
            threadpool->shutdown = true;
            cnd_broadcast(&threadpool->cv_work);

            for(int j = 0; j < i; ++j) {
                thrd_join(threadpool->threads[j], NULL);
            }

            free(threadpool->threads);
            free(threadpool->worker_data);
            mtx_destroy(&threadpool->lock);
            cnd_destroy(&threadpool->cv_work);
            cnd_destroy(&threadpool->cv_done);
            return false;
        }
    }

    return true;
}

void threadpool_run(ThreadPool *threadpool, Pendulum *pendulums) {
    mtx_lock(&threadpool->lock);
    threadpool->work_ready = true;
    threadpool->jobs_remaining = threadpool->num_threads;
    threadpool->pendulums = pendulums;

    /* Reset local max values for a new run — MUST happen BEFORE broadcast */
    for(int i = 0; i < threadpool->num_threads; ++i) {
        threadpool->worker_data[i].local_max_ang_vel = 0.0;
    }

    cnd_broadcast(&threadpool->cv_work);
    mtx_unlock(&threadpool->lock);

    /* Wait for workers to finish */
    mtx_lock(&threadpool->lock);
    while(threadpool->jobs_remaining > 0) {
        cnd_wait(&threadpool->cv_done, &threadpool->lock);
    }
    mtx_unlock(&threadpool->lock);
}

double threadpool_get_max_ang_vel(ThreadPool *threadpool) {
    double max_val = 0.0;
    for(int t = 0; t < threadpool->num_threads; ++t) {
        if(threadpool->worker_data[t].local_max_ang_vel > max_val)
            max_val = threadpool->worker_data[t].local_max_ang_vel;
    }
    return max_val;
}

bool threadpool_quit(ThreadPool *threadpool) {
    if(!threadpool) return false;

    mtx_lock(&threadpool->lock);
    threadpool->shutdown = true;
    cnd_broadcast(&threadpool->cv_work);
    mtx_unlock(&threadpool->lock);

    for(int t = 0; t < threadpool->num_threads; ++t) {
        thrd_join(threadpool->threads[t], NULL);
    }

    free(threadpool->threads);
    free(threadpool->worker_data);

    mtx_destroy(&threadpool->lock);
    cnd_destroy(&threadpool->cv_work);
    cnd_destroy(&threadpool->cv_done);

    return true;
}
