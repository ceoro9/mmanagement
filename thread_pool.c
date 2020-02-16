#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "list.h"


/**
 * @struct thread_pool_t
 */
typedef struct {

    // Signifies if pool is opened
    bool is_opened;

    // Number of thread workers in pool
    unsigned int number_of_threads;

    // Array of thread worker ids
    pthread_t *thread_ids;

    // List of tasks to execute
    list_t *tasks_list;

    pthread_mutex_t *add_task_mutex;

} thread_pool_t;

/**
 * @struct thread_pool_task_t
 */
typedef struct {

    // Tasks function to execute by worker
    void (*func_to_execute) (void);

} thread_pool_task_t;

/**
 * @struct thread_pool_worker_input_t
 */
typedef struct thread_pool_worker_input_ {

    // Signifies if pool is opened
    bool *is_pool_opened;

    // Barrier to wait until all workers are set up
    pthread_barrier_t *thread_barrier_ptr;

} thread_pool_worker_input_t;


/**
 * @function inits thread pool task to execute
 */
thread_pool_task_t *init_thread_pool_task(void (*func_to_execute) (void)) {

    thread_pool_task_t *result = (thread_pool_task_t*) malloc(sizeof(thread_pool_task_t));

    if (!result) {
      return NULL;
    }

    result->func_to_execute = func_to_execute;

    return result;
}

/**
 * @function thread_pool_worker
 */
void *thread_pool_worker(void *data) {

    thread_pool_worker_input_t *worker_input = (thread_pool_worker_input_t*) data;
    bool *is_pool_opened = worker_input->is_pool_opened; 

    printf("#%d thread is ready\n", pthread_self());
    fflush(stdout);

    pthread_barrier_wait(worker_input->thread_barrier_ptr);
    // data is longer valid, because it was freed

    while (*is_pool_opened) {

        // TODO: wait for tasks to execute

        pthread_yield();
    }

    printf("#%d thread is finished\n", pthread_self());
    fflush(stdout);

    return NULL;
}


/**
 * @function inits thread pool
 */
thread_pool_t *init_thread_pool(unsigned int number_of_threads) {

    thread_pool_t *result_thread_pool = (thread_pool_t*) malloc(sizeof(thread_pool_t));

    if (!result_thread_pool) {
        return NULL;
    }

    result_thread_pool->is_opened = true;
    result_thread_pool->number_of_threads = number_of_threads;

    result_thread_pool->thread_ids = (pthread_t*) malloc(sizeof(pthread_t) * number_of_threads);
    if (!result_thread_pool->thread_ids) {
        free(result_thread_pool);
        return NULL;
    }

    result_thread_pool->tasks_list = init_list();
    if (!result_thread_pool->tasks_list) {
        free(result_thread_pool->thread_ids);
        free(result_thread_pool);
        return NULL;
    }

    result_thread_pool->add_task_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    if (!result_thread_pool->add_task_mutex) {
        close_list(result_thread_pool->tasks_list);
        free(result_thread_pool->thread_ids);
        free(result_thread_pool);
        return NULL;
    }

    pthread_mutex_init(result_thread_pool->add_task_mutex, NULL);

    // barrier is onle used on initialization phase
    pthread_barrier_t thread_barrier;
    pthread_barrier_init(&thread_barrier, NULL, number_of_threads + 1);

    thread_pool_worker_input_t *worker_input = (thread_pool_worker_input_t*) malloc(sizeof(thread_pool_worker_input_t));

    if (!worker_input) {
        free(result_thread_pool->thread_ids);
        free(result_thread_pool);
        return NULL;
    }

    worker_input->is_pool_opened = &result_thread_pool->is_opened;
    worker_input->thread_barrier_ptr = &thread_barrier;

    // Start workers
    for (int i = 0; i < number_of_threads; ++i) {
        pthread_create(result_thread_pool->thread_ids + i, NULL, &thread_pool_worker, worker_input);
    }

    pthread_barrier_wait(&thread_barrier);
    pthread_barrier_destroy(&thread_barrier);

    free(worker_input);

    return result_thread_pool;
}


/**
 * @function Frees thread pool's memory
 */
void destroy_thread_pool(thread_pool_t *tp) {
  if (!tp) {
    return;
  }
  free(tp->tasks_list);
  free(tp->thread_ids);
  free(tp);
}


/**
 * @function Wait until all workers are down and frees thread pool's memory
 */
void close_thread_pool(thread_pool_t *tp) {

    if (!tp || !tp->is_opened) {
        return;
    }

    tp->is_opened = false;

    for (int i = 0; i < tp->number_of_threads; ++i) {
      pthread_join(*(tp->thread_ids + i), NULL);  // TODO: handle status
    }

    destroy_thread_pool(tp);

    return;
}


int add_task_to_thread_pool(thread_pool_t *thread_pool, void(*task_func)(void*)) {

    thread_pool_task_t *task = init_thread_pool_task(task_func);
    list_item_data_t *item_data = init_list_item_data(task, NULL);

    if (!item_data) {
        return -1;
    }

    pthread_mutex_lock(thread_pool->add_task_mutex);
    add_item_to_list(thread_pool->tasks_list, item_data);
    pthread_mutex_unlock(thread_pool->add_task_mutex);

    return 0;
}

int main() {

    thread_pool_t *tp = init_thread_pool(10);
    close_thread_pool(tp);

    return 0;
}

