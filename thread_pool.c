#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include "list.h"


/**
 * @typedef task function to execute in thread pool
 */
typedef void (*task_func_t)(void*);


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

    // Mutex to sync access to add task to thread pool
    pthread_mutex_t *add_task_mutex;

    /*
     * Resources, that are shared by workers.
     * Should be freed on thread pool close
     */

    // Condition to report that there is a task in list
    pthread_cond_t *thread_cond_ptr;

    // Condition mutex
    pthread_mutex_t *thread_mutex_ptr;

    /*
     * --- END ---
     */

} thread_pool_t;


/**
 * @struct thread_pool_task_t
 */
typedef struct {

    // Tasks function to execute by worker
    task_func_t func;

} thread_pool_task_t;


/**
 * @struct thread_pool_worker_input_t
 */
typedef struct thread_pool_worker_input_ {

    // Signifies if pool is opened
    bool *is_pool_opened;

    // List with tasks to execute
    list_t *tasks_list; // tasks_list<thread_pool_task_t>

    // Barrier to wait until all workers are set up
    pthread_barrier_t *thread_barrier_ptr;

    // Condition to report that there is a task in list 
    pthread_cond_t *thread_cond_ptr;

    // Condition mutex
    pthread_mutex_t *thread_mutex_ptr;

} thread_pool_worker_input_t;


/**
 * @function inits thread pool task to execute
 */
thread_pool_task_t *init_thread_pool_task(task_func_t func_to_execute) {

    thread_pool_task_t *result = (thread_pool_task_t*) malloc(sizeof(thread_pool_task_t));

    if (!result) {
      return NULL;
    }

    result->func = func_to_execute;

    return result;
}


/**
 * @function frees thread pool task memory
 */
void free_thread_pool_task(thread_pool_task_t *tp_task) {
  free(tp_task);
}


/**
 * @function thread_pool_worker
 */
void *thread_pool_worker(void *data) {

    thread_pool_worker_input_t *worker_input = (thread_pool_worker_input_t*) data;

    // TODO: make shallow copy of worker_input
    bool *is_pool_opened = worker_input->is_pool_opened;
    pthread_cond_t *tasks_cond = worker_input->thread_cond_ptr;
    pthread_mutex_t *tasks_mutex = worker_input->thread_mutex_ptr;
    list_t *tasks_list = worker_input->tasks_list;

    printf("#%d thread is ready\n", pthread_self());
    fflush(stdout);

    pthread_barrier_wait(worker_input->thread_barrier_ptr);
    // data is longer valid, because it was freed

    while (*is_pool_opened) {

        while (is_list_empty(tasks_list) && *is_pool_opened) {
            pthread_cond_wait(tasks_cond, tasks_mutex);
        }

        // pool was closed
        if (!(*is_pool_opened)) {
            pthread_mutex_unlock(tasks_mutex);
            break;
        }

        list_item_t *list_item   = get_item_from_head_of_list(tasks_list);
        thread_pool_task_t *task = (thread_pool_task_t*) get_list_item_data(list_item);
        task_func_t task_func    = task->func;

        // remove task from list and free its memory
        remove_item_from_list(tasks_list, list_item);

        pthread_mutex_unlock(tasks_mutex);

        (task_func)(NULL); // TODO: pass user input
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

    //
    // condition and its mutex are used during all life time of thread pool
    pthread_cond_t *thread_cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));

    if (!thread_cond) {
      close_list(result_thread_pool->tasks_list);
      free(result_thread_pool->thread_ids);
      free(result_thread_pool);
      return NULL;
    }

    pthread_mutex_t *thread_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

    if (!thread_cond) {
      free(thread_cond);
      close_list(result_thread_pool->tasks_list);
      free(result_thread_pool->thread_ids);
      free(result_thread_pool);
      return NULL;
    }

    pthread_cond_init(thread_cond, NULL);
    pthread_mutex_init(thread_mutex, NULL);

    result_thread_pool->thread_cond_ptr  = thread_cond;
    result_thread_pool->thread_mutex_ptr = thread_mutex;

    // Init worker input
    thread_pool_worker_input_t *worker_input = (thread_pool_worker_input_t*) malloc(sizeof(thread_pool_worker_input_t));
    if (!worker_input) {
      free(thread_cond);
      free(thread_mutex);
      close_list(result_thread_pool->tasks_list);
      free(result_thread_pool->thread_ids);
      free(result_thread_pool);
      return NULL;
    }

    worker_input->is_pool_opened     = &result_thread_pool->is_opened;
    worker_input->thread_barrier_ptr = &thread_barrier;
    worker_input->thread_cond_ptr    = thread_cond;
    worker_input->thread_mutex_ptr   = thread_mutex;
    worker_input->tasks_list         = result_thread_pool->tasks_list;

    // Start workers
    for (int i = 0; i < number_of_threads; ++i) {
        pthread_create(result_thread_pool->thread_ids + i, NULL, &thread_pool_worker, worker_input);
    }

    // Wait until all workers are up
    pthread_barrier_wait(&thread_barrier);
    pthread_barrier_destroy(&thread_barrier);

    // Each worker will shallow copy input, so we free this memory
    free(worker_input);

    return result_thread_pool;
}


/**
 * @function Frees all thread pool's resources
 */
void destroy_thread_pool(thread_pool_t *tp) {

  if (!tp) {
    return;
  }

  pthread_mutex_destroy(tp->add_task_mutex);
  pthread_mutex_destroy(tp->thread_mutex_ptr);
  pthread_cond_destroy(tp->thread_cond_ptr);
  close_list(tp->tasks_list);

  free(tp->thread_cond_ptr);
  free(tp->thread_mutex_ptr);
  free(tp->add_task_mutex);
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
    pthread_cond_broadcast(tp->thread_cond_ptr); // wake up all awaiting workers

    for (int i = 0; i < tp->number_of_threads; ++i) {
      pthread_join(*(tp->thread_ids + i), NULL);  // TODO: handle status
    }

    destroy_thread_pool(tp);

    return;
}


int add_task_to_thread_pool(thread_pool_t *thread_pool, void(*task_func)(void*)) {

    thread_pool_task_t *task = init_thread_pool_task(task_func);

    if (!task) {
        return -1;
    }

    // once item is removed from list, task memory will be freed
    list_item_data_t *item_data = init_list_item_data(task, free_thread_pool_task);

    if (!item_data) {
        free_thread_pool_task(task);
        return -1;
    }

    pthread_mutex_lock(thread_pool->add_task_mutex);
    add_item_to_tail_of_list(thread_pool->tasks_list, item_data);
    pthread_cond_signal(thread_pool->thread_cond_ptr);
    pthread_mutex_unlock(thread_pool->add_task_mutex);

    return 0;
}


void test_task_func_1(void *input) {
    printf("Hello world from #%d thread\n", pthread_self());
    fflush(stdout);
}


void test_task_func_2(void *input) {
  printf("Bye world from #%d thread\n", pthread_self());
  fflush(stdout);
}


int main() {

    thread_pool_t *tp = init_thread_pool(10);

    add_task_to_thread_pool(tp, &test_task_func_1);
    add_task_to_thread_pool(tp, &test_task_func_1);
    add_task_to_thread_pool(tp, &test_task_func_2);
    add_task_to_thread_pool(tp, &test_task_func_2);

    sleep(3);  // sleep for 3 seconds
    close_thread_pool(tp);

    return 0;
}

