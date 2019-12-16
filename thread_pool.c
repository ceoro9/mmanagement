#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct thread_pool_t_ {
    bool is_active;
    unsigned int number_of_threads;
    pthread_t *thread_ids;
} thread_pool_t;

void thread_pool_worker(void *data) {

    #ifdef _POSIX_BARRIERS

    pthread_barrier_t *thread_barrier_ptr = (pthread_barrier_t*) data;
    pthread_barrier_wait(thread_barrier_ptr);

    #else

    printf("ERROR: NO SUPPORT OF POSIX BARRIERS");
    exit(1);

    #endif

    // TODO: while thread_pool is active
    while (true) {

    }
} 

thread_pool_t *create_thread_pool(unsigned int number_of_threads) {

    thread_pool_t *result_thread_pool = malloc(sizeof(thread_pool_t));

    if (result_thread_pool == NULL) {
        return NULL;
    }

    result_thread_pool->is_active = true;
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
    thread_pool->is_active = false;
    // TODO: wait until all thread are done
    free(thread_pool->thread_ids);
    free(thread_pool);
}

int main() {
    return 0;
}