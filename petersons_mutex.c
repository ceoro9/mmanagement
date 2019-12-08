#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

typedef struct thread_flags_t_ {
    size_t size;
    bool *array;
} thread_flags_t;

thread_flags_t *thread_flags = NULL;
int thread_turn = -1;

int result_answer_to_increment = 0; // to demonstrate mutex proof-of-consept

int get_max_pid_value() {
    int max_pid_value_result;
    FILE *max_pid_file_ptr;

    if ((max_pid_file_ptr = fopen("/proc/sys/kernel/pid_max", "r")) == NULL) {
        printf("Cannot open max_pid_file\n");
        exit(0);
    }

    fscanf(max_pid_file_ptr, "%d", &max_pid_value_result);

    fclose(max_pid_file_ptr);

    return max_pid_value_result;
}

void lock_init() {

    if (thread_flags == NULL) {

        int max_pid_value = get_max_pid_value();

        thread_flags = (thread_flags_t*) malloc(sizeof(thread_flags_t));

        if (thread_flags == NULL) {
            printf("Not enough memory to spot thread flags\n");
            exit(0);
        }

        size_t thread_flags_size = max_pid_value + 1;

        thread_flags->array = (bool*) malloc(sizeof(bool) * thread_flags_size);

        for (size_t i = 0; i < thread_flags_size; ++i) {
            thread_flags->array[i] = 0;
        }

        if (thread_flags->array == NULL) {
            printf("Not enough memory to spot thread flags array in size of %d", thread_flags_size);
            exit(0);
        }

        thread_flags->size = thread_flags_size;
    }

    thread_turn = 0;

    return;
}

void lock_destroy() {
    // clear array of thread flags
    free(thread_flags->array);
    thread_flags->array = NULL;

    // clea thread flag data structure
    free(thread_flags);
    thread_flags = NULL;
}

void lock(int self) {

    thread_flags->array[self] = true;

    for (size_t thread_pid_to_check = 1; thread_pid_to_check <= thread_flags->size; ++thread_pid_to_check) {

        if (thread_pid_to_check == self) {
            continue;
        }

        thread_turn = thread_pid_to_check;

        while (
            thread_flags->array[thread_pid_to_check] == true &&
            thread_turn == thread_pid_to_check
        );
    }

    return;
}

void unlock(int self) {
    thread_flags->array[self] = false;
}

void *perform_incrementing(void *args) {

    int number_to_increment = (int*) args;
    int self_pid = (int) getpid();

    lock(self_pid);

    for (int i = 0; i < number_to_increment; ++i) {
        ++result_answer_to_increment;
    }

    unlock(self_pid);
}

int main() {

    pthread_t p1, p2;

    lock_init();

    pthread_create(&p1, NULL, perform_incrementing, (void*)5);
    pthread_create(&p2, NULL, perform_incrementing, (void*)10);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    lock_destroy();

    printf("Result answer = %d", result_answer_to_increment);

    return 0;    
}