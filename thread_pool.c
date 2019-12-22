#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

// ThreadPool ...
typedef struct thread_pool_t_ {
    
    // Signifies if pool is opened
    bool is_opened;
    
    // Number of thread workers in pool
    unsigned int number_of_threads;

    // Array of thread worker ids
    pthread_t *thread_ids;

} thread_pool_t;

// ThreadPoolWorkerInput ...
typedef struct thread_pool_worker_input_ {

    // Signifies if pool is opened
    bool is_pool_opened;
    
    // Barrier to wait until all workers are set up
    #ifdef _POSIX_BARRIERS
    pthread_barrier_t *thread_barrier_ptr;
    #endif
    
    // Condition to get a function to execute 
    pthread_cond_t *thread_cond_ptr;
    
    // Mutex to sync access to function
    pthread_mutex_t *thread_mutex_ptr;

    // Function to execute by one of workers
    void (*function_to_execute_ptr)(void*);

} thread_pool_worker_input_t;

void thread_pool_worker(void *data) {

    #ifdef _POSIX_BARRIERS

    thread_pool_worker_input_t *worker_input = (thread_pool_worker_input_t*) data;
    
    pthread_barrier_wait(worker_input->thread_cond_ptr);

    while (worker_input->is_pool_opened) {

	pthread_mutex_lock(worker_input->thread_mutex_ptr);
	
	while (worker_input->function_to_execute_ptr == NULL) {
	    pthread_cond_wait(
		worker_input->thread_cond_ptr,
		worker_input->thread_mutex_ptr
	    );
	}

	worker_input->function_to_execute_ptr();
	worker_input->function_to_execute_ptr = NULL;

	pthread_mutex_unlock(worker_input->thread_mutex_ptr);
    }

    #else

    printf("ERROR: NO SUPPORT OF POSIX BARRIERS");
    exit(1);

    #endif
} 

thread_pool_t *create_thread_pool(unsigned int number_of_threads) {

    thread_pool_t *result_thread_pool = malloc(sizeof(thread_pool_t));

    if (result_thread_pool == NULL) {
        return NULL;
    }

    result_thread_pool->is_opened = true;
    result_thread_pool->number_of_threads = number_of_threads;
    result_thread_pool->thread_ids = (pthread_t*) malloc(sizeof(pthread_t) * number_of_threads);

    if (result_thread_pool->thread_ids == NULL) {
        free(result_thread_pool);
        return NULL;
    }

    #ifdef _POSIX_BARRIERS

    pthread_barrier_t thread_barrier;

    pthread_barrier_init(&thread_barrier, NULL, number_of_threads + 1);

    for (int i = 0; i < number_of_threads; ++i) {
        pthread_create(result_thread_pool->thread_ids[i], thread_pool_worker, NULL, &thread_barrier);
    }

    pthread_barrier_wait(&thread_barrier);
    pthread_barrier_destroy(&thread_barrier);
    
    #else

    printf("ERROR: NO SUPPORT OF POSIX BARRIERS");
    exit(1);

    #endif
    
    return result_thread_pool;
}

void destroy_thread_pool(thread_pool_t *thread_pool) {
    thread_pool->is_opened = false;
    // TODO: wait until all thread are done
    free(thread_pool->thread_ids);
    free(thread_pool);
}

int main() {
    return 0;
}
