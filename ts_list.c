#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "list.h"
#define LAMBDA(c_) ({ c_ _;})


char *itoa(int num, char *str) {
  if(!str) {
    return NULL;
  }
  sprintf(str, "%d", num);
  return str;
}


typedef struct {

  pthread_mutex_t add_head_mutex;
  pthread_mutex_t add_tail_mutex;
  pthread_mutex_t remove_mutex;

  list_t *origin_list;

} ts_list_t;


ts_list_t *init_ts_list() {

  list_t *list = init_list();

  if (!list) {
    return NULL;
  }

  ts_list_t *ts_list = (ts_list_t*) malloc(sizeof(ts_list_t));

  if (!ts_list) {
    close_list(list);
    return NULL;
  }

  pthread_mutex_init(&ts_list->add_head_mutex, NULL);
  pthread_mutex_init(&ts_list->add_tail_mutex, NULL);
  pthread_mutex_init(&ts_list->remove_mutex, NULL);

  ts_list->origin_list = list;

  return ts_list;
}


void lock_all_mutexes(ts_list_t *list) {
  pthread_mutex_lock(&list->add_head_mutex);
  pthread_mutex_lock(&list->add_tail_mutex);
  pthread_mutex_lock(&list->remove_mutex);
}


void unlock_all_mutexes(ts_list_t *list) {
  pthread_mutex_unlock(&list->add_head_mutex);
  pthread_mutex_unlock(&list->add_tail_mutex);
  pthread_mutex_unlock(&list->remove_mutex);
}


void destroy_all_mutexes(ts_list_t *list) {
  pthread_mutex_destroy(&list->add_head_mutex);
  pthread_mutex_destroy(&list->add_tail_mutex);
  pthread_mutex_destroy(&list->remove_mutex);
}


void TS_close_list(ts_list_t *list) {

  lock_all_mutexes(list);
  close_list(list->origin_list);
  unlock_all_mutexes(list);

  destroy_all_mutexes(list);

  free(list);
}


list_item_t *TS_add_item_to_head_of_list(ts_list_t *list, list_item_data_t *item_data) {

  pthread_mutex_lock(&list->add_head_mutex);
  list_item_t *new_item = add_item_to_head_of_list(list->origin_list, item_data);
  pthread_mutex_unlock(&list->add_head_mutex);

  return new_item;
}


list_item_t *TS_add_item_to_tail_of_list(ts_list_t *list, list_item_data_t *item_data) {

  list_item_t *new_item;

  pthread_mutex_lock(&list->add_tail_mutex);

  if (is_list_empty(list->origin_list)) {

    pthread_mutex_lock(&list->add_head_mutex);

    //
    // No one can add item to list when this block runs
    //
    if (is_list_empty(list->origin_list)) {
      new_item = add_item_to_head_of_list(list->origin_list, item_data);
    } else {
      new_item = add_item_to_tail_of_list(list->origin_list, item_data);
    }

    pthread_mutex_unlock(&list->add_head_mutex);

  } else {
    new_item = add_item_to_tail_of_list(list->origin_list, item_data);
  }

  pthread_mutex_unlock(&list->add_tail_mutex);

  return new_item;
}


static inline int c_is_first_item(ts_list_t *list, list_item_t *item) {
  return list->origin_list->head->next == item;
}

static inline int c_is_last_item(ts_list_t *list, list_item_t *item) {
  return list->origin_list->last_item == item;
}


int TS_remove_item_from_list(ts_list_t *list, list_item_t *searched_item) {

  int result;

  //
  // Handlers
  //
  int (*handle_when_first_item) (ts_list_t*, list_item_t*) = LAMBDA(int _(ts_list_t *list, list_item_t *item) {
    int result = remove_item_from_list(list->origin_list, searched_item);
    pthread_mutex_unlock(&list->add_head_mutex);
    return result;
  });

  int (*handle_when_last_item) (ts_list_t*, list_item_t*) = LAMBDA(int _(ts_list_t *list, list_item_t *item) {
    int result = remove_item_from_list(list->origin_list, searched_item);
    pthread_mutex_unlock(&list->add_tail_mutex);
    return result;
  });

  pthread_mutex_lock(&list->remove_mutex);

  int is_first_item = c_is_first_item(list, searched_item);
  int is_last_item  = c_is_last_item(list, searched_item);

  if (is_first_item && is_last_item) {

    pthread_mutex_lock(&list->add_head_mutex);
    pthread_mutex_lock(&list->add_tail_mutex);

    // Test if this condition is still true
    // to determine more efficient way to unlock add mutexes
    int new_is_first_item = c_is_first_item(list, searched_item);
    int new_is_last_item  = c_is_last_item(list, searched_item);

    if (new_is_first_item && new_is_last_item) {

      result = remove_item_from_list(list->origin_list, searched_item);
      pthread_mutex_unlock(&list->add_tail_mutex);
      pthread_mutex_unlock(&list->add_head_mutex);

    } else if (new_is_first_item) {

      pthread_mutex_unlock(&list->add_tail_mutex);
      result = handle_when_first_item(list, searched_item);

    } else if (new_is_last_item) {

      pthread_mutex_unlock(&list->add_head_mutex);
      result = handle_when_last_item(list, searched_item);

    } else {

      result = remove_item_from_list(list->origin_list, searched_item);

    }
  } else if (is_first_item) {

    pthread_mutex_lock(&list->add_head_mutex);

    // Test if this condition is still true
    // to determine more efficient way to unlock add mutexes
    int new_is_first_item = c_is_first_item(list, searched_item);

    if (new_is_first_item) {
      result = handle_when_first_item(list, searched_item);
    } else {
      pthread_mutex_unlock(&list->add_head_mutex);
      result = remove_item_from_list(list->origin_list, searched_item);
    }
  } else if (is_last_item) {

    pthread_mutex_lock(&list->add_tail_mutex);

    // Test if this condition is still true
    // to determine more efficient way to unlock add mutexes
    int new_is_last_item  = c_is_last_item(list, searched_item);

    if (new_is_last_item) {
      result = handle_when_last_item(list, searched_item);
    } else {
      pthread_mutex_unlock(&list->add_tail_mutex);
      result = remove_item_from_list(list->origin_list, searched_item);
    }
  } else {
    result = remove_item_from_list(list->origin_list, searched_item);
  }

  pthread_mutex_unlock(&list->remove_mutex);

  return result;
}


// ------------------------------------------------------
// ------------------------ Tests -----------------------
// ------------------------------------------------------


const int NUMBER_OF_THREADS = 100;
const int EACH_THREAD_CHUNK = 10;

typedef struct {
  ts_list_t *list;
  int start_counter;
  int end_counter;
  int add_to_head;
} worker_input_t;


void *test_add_worker(void *_input) {

  worker_input_t *input = (worker_input_t*) _input;

  for (int i = input->start_counter; i < input->end_counter; ++i) {

    char *data = malloc(20);
    list_item_data_t *item_data = init_list_item_data(itoa(i, data), free);

    if (input->add_to_head) {
      TS_add_item_to_head_of_list(input->list, item_data);
    } else {
      TS_add_item_to_tail_of_list(input->list, item_data);
    }
  }

  free(_input);

  return NULL;
}


void *test_add_and_remove_worker(void *_input) {

  list_item_t *current_item;
  worker_input_t *input = (worker_input_t*) _input;

  for (int i = input->start_counter; i < input->end_counter; ++i) {

    char *data = malloc(20);
    list_item_data_t *item_data = init_list_item_data(itoa(i, data), free);

    if (input->add_to_head) {
      current_item = TS_add_item_to_head_of_list(input->list, item_data);
    } else {
      current_item = TS_add_item_to_tail_of_list(input->list, item_data);
    }

    double time_to_sleep = (rand() % 100) * 0.001; // 0 - 0.099
    sleep(time_to_sleep);

    TS_remove_item_from_list(input->list, current_item);
  }

  free(_input);

  return NULL;
}


ts_list_t *spawn_and_join_workers(void *(*worker_func)(void*)) {

  ts_list_t *list = init_ts_list();


  pthread_t *thread_ids = (pthread_t*) malloc(sizeof(pthread_t) * NUMBER_OF_THREADS);

  for (int i = 0; i < NUMBER_OF_THREADS; ++i) {

    worker_input_t *input = (worker_input_t*) malloc(sizeof(worker_input_t));
    input->list = list;
    input->start_counter = i * EACH_THREAD_CHUNK;
    input->end_counter   = input->start_counter + EACH_THREAD_CHUNK;
    input->add_to_head   = rand() % 2;

    pthread_create(thread_ids + i, NULL, worker_func, (void*) input);
  }


  for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
    pthread_join(*(thread_ids + i), NULL);
  }

  free(thread_ids);

  return list;
}


int test_add_head_tail() {

  ts_list_t *list = spawn_and_join_workers(test_add_worker);

  int counter = 0;
  list_item_t *current_item = list->origin_list->head->next;

  while (current_item != list->origin_list->tail) {
    ++counter;
    current_item = current_item->next;
  }

  TS_close_list(list);

  if (counter != EACH_THREAD_CHUNK * NUMBER_OF_THREADS) {
    printf("test_add_head_tail: Error. Counter = %d\n", counter);
    return 1;
  }

  printf("test_add_head_tail: Success\n");

  return 0;
}

int test_add_and_remove_operations() {
  ts_list_t *list = spawn_and_join_workers(test_add_and_remove_worker);

  if (is_list_empty(list->origin_list)) {
    printf("test_add_and_remove: Success\n");
    return 0;
  } else {
    printf("test_add_and_remove: Failure\n");
    return 1;
  }
}


int main() {
  const int test_result_1 = test_add_head_tail();
  const int test_result_2 = test_add_and_remove_operations();
  return test_result_1 && test_result_2;
}

