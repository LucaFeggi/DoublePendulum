#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "pendulum.h"
#include <stdbool.h>

#include <threads.h>

typedef struct{
    int thread_id;
    int start_index;
    int end_index;
    double local_max_ang_vel;
    struct ThreadPool *threadpool;
} WorkerData;

typedef struct ThreadPool{
    thrd_t *threads;
    WorkerData *worker_data;

    mtx_t lock;
    cnd_t cv_work;
    cnd_t cv_done;

    bool work_ready;
    bool shutdown;
    int jobs_remaining;
    int num_threads;

    Pendulum *pendulums;
} ThreadPool;

bool threadpool_init(ThreadPool *threadpool);
bool threadpool_quit(ThreadPool *threadpool);
void threadpool_run(ThreadPool *threadpool, Pendulum *pendulums);
double threadpool_get_max_ang_vel(ThreadPool *threadpool);

#endif // THREADPOOL_H
