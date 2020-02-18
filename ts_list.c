#include <stdlib.h>
#include <pthread.h>
#include "list.h"


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

  ts_list_t *ts_list = (ts_list_t*) malloc(sizeof(ts_list));

  pthread_mutex_init(&ts_list->add_head_mutex, NULL);
  pthread_mutex_init(&ts_list->add_tail_mutex, NULL);
  pthread_mutex_init(&ts_list->remove_mutex, NULL);

  ts_list->origin_list = list;

  return ts_list;
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


int TS_remove_item_from_list(ts_list_t *list, list_item_t *searched_item) {

  int result;

  pthread_mutex_lock(&list->remove_mutex);

  int is_first_item = list->origin_list->head->next == searched_item;
  int is_last_item  = searched_item->next == list->origin_list->tail;

  if (is_first_item && is_last_item) {

    pthread_mutex_lock(&list->add_head_mutex);
    pthread_mutex_lock(&list->add_tail_mutex);
    // TODO: check one more time
    result = remove_item_from_list(list->origin_list, searched_item);
    pthread_mutex_unlock(&list->add_tail_mutex);
    pthread_mutex_unlock(&list->add_head_mutex);

  } else if (is_first_item) {

    pthread_mutex_lock(&list->add_head_mutex);
    // TODO: check one more time
    result = remove_item_from_list(list->origin_list, searched_item);
    pthread_mutex_unlock(&list->add_head_mutex);

  } else if (is_last_item) {

    pthread_mutex_lock(&list->add_tail_mutex);
    // TODO: check one more time
    result = remove_item_from_list(list->origin_list, searched_item);
    pthread_mutex_unlock(&list->add_tail_mutex);

  } else {
    result = remove_item_from_list(list->origin_list, searched_item);
  }

  pthread_mutex_unlock(&list->remove_mutex);

  return result;
}


int main() {

  return 0;
}
