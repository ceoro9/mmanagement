#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define THREAD_IS_RUNNING_STATUS_VALUE 0
#define THREAD_DESIRE_EXCLUSIVE_ACCESS 1
#define MAX_INCREMENT_VALUE 50000

typedef struct thread_flags_t_ {
    size_t size;
    volatile bool *array;
} thread_flags_t;

// Array of thread flags, which signifies thread desire to acquire resource
thread_flags_t *thread_flags = NULL;

// Next thread in turn
volatile size_t thread_turn = -1;

// Counter to contoler id of spotted thread
size_t next_spotted_thread_id = 0;

// Counter to demonstrate mutex proof-of-concept
int result_answer_to_increment = 0; 

size_t get_max_pid_value() {

    size_t max_pid_value_result;
    FILE *max_pid_file_ptr;

    if ((max_pid_file_ptr = fopen("/proc/sys/kernel/pid_max", "r")) == NULL) {
        printf("Cannot open max_pid_file\n");
        exit(0);
    }

    fscanf(max_pid_file_ptr, "%zu", &max_pid_value_result);

    fclose(max_pid_file_ptr);

    return max_pid_value_result;
}

void lock_init() {

    if (thread_flags == NULL) {

        // for simplicity take max pid as size of thread flag array
        size_t max_pid_value = get_max_pid_value();

        thread_flags = (thread_flags_t*) malloc(sizeof(thread_flags_t));

        if (thread_flags == NULL) {
            printf("Not enough memory to spot thread flags\n");
            exit(0);
        }

        size_t thread_flags_size = max_pid_value + 1;   

        thread_flags->array = (bool*) malloc(sizeof(bool) * thread_flags_size);

        if (thread_flags->array == NULL) {
            printf("Not enough memory to spot thread flags array in size of %d", thread_flags_size);
            exit(0);
        }

        for (size_t i = 0; i < thread_flags_size; ++i) {
            thread_flags->array[i] = THREAD_IS_RUNNING_STATUS_VALUE;
        }

        thread_flags->size = thread_flags_size;
    }

    thread_turn = THREAD_IS_RUNNING_STATUS_VALUE;
}

void lock_destroy() {
    // clear array of thread flags
    free((bool*)thread_flags->array);
    thread_flags->array = NULL;

    // clear thread flag data structure
    free((thread_flags_t*)thread_flags);
    thread_flags = NULL;
}

void lock(size_t self) {

    thread_flags->array[self] = THREAD_DESIRE_EXCLUSIVE_ACCESS;

    for (size_t thread_pid_to_check = 0; thread_pid_to_check < thread_flags->size; ++thread_pid_to_check) {

        // do not check the self thread pid
        if (thread_pid_to_check == self) {
            continue;
        }

        // do not check thread pid, that does not desire access
        if (thread_flags->array[thread_pid_to_check] != THREAD_DESIRE_EXCLUSIVE_ACCESS) {
            continue;
        }

        thread_turn = thread_pid_to_check;

        while (
            thread_flags->array[thread_pid_to_check] == THREAD_DESIRE_EXCLUSIVE_ACCESS &&
            thread_turn == thread_pid_to_check
        );
    }

    return;
}

void unlock(size_t self) {
    thread_flags->array[self] = THREAD_IS_RUNNING_STATUS_VALUE;
}

void *perform_incrementing(void *args) {

    size_t self_thread_pid = (size_t*) args;

    printf("Thread #%ud started\n", self_thread_pid);

    lock(self_thread_pid);

    for (int i = 0; i < MAX_INCREMENT_VALUE; ++i) {
        ++result_answer_to_increment;
    }

    unlock(self_thread_pid);

    printf("Thread #%d finished\n", self_thread_pid);
}

pthread_t *start_increment_thread() {

    pthread_t *result_pid = (pthread_t*) malloc(sizeof(pthread_t));

    pthread_create(result_pid, NULL, perform_incrementing, (void*)next_spotted_thread_id++);

    return result_pid;
}

int main() {

    lock_init();

    pthread_t *p1 = start_increment_thread();
    pthread_t *p2 = start_increment_thread();

    pthread_join(*p1, NULL);
    pthread_join(*p2, NULL);

    lock_destroy();

    free(p1);
    free(p2);

    int expected_result_answer = MAX_INCREMENT_VALUE * 2;

    printf("Result = %d. Expected = %d\n", result_answer_to_increment, expected_result_answer);

    printf(expected_result_answer == result_answer_to_increment ? "Success\n" : "Failure\n");

    return 0;    
}