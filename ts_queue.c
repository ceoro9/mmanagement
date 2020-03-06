#include <stdlib.h>

#ifndef ST_LIST_
#define ST_LIST_
#include "list.h"
#endif

#ifndef ST_TS_LIST_
#define ST_TS_LIST_
#include "ts_list.h"
#endif

#ifndef ST_TS_QUEUE_
#define ST_TS_QUEUE_
#include "ts_queue.h"
#endif


/**
 * @function inits thread-safe queue
 * @returns  pointer to created queue
 */
ts_queue_t *TS_init_queue() {

  ts_list_t *list = init_ts_list();

  if (!list) {
    return NULL;
  }

  ts_queue_t *queue = (ts_queue_t*) malloc(sizeof(ts_queue_t));

  if (!queue) {
    TS_close_list(list);
    return NULL;
  }

  queue->list = list;

  return queue;
}


/**
 * @function frees queue memory
 * @returns  void
 */
void TS_close_queue(ts_queue_t *queue) {
  TS_close_list(queue->list);
  free(queue);
}


/**
 * @function push data to queue
 * @returns  0 - success, other - error
 */
int TS_push_to_queue(ts_queue_t *queue, void *data) {

  list_item_data_t *item_data = init_list_item_data(data, DO_NOTHING_FUNC);

  if (!item_data) {
    return 1;
  }

  list_item_t *item = TS_add_item_to_head_of_list(queue->list, item_data);

  if (!item) {
    free_list_item_data(item_data);
    return 1;
  }

  return 0;
}


/**
 * @function pops element from queue
 * @returns  NULL if queue is empty and data pointer if not
 */
void *TS_pop_from_queue(ts_queue_t *queue) {
  // TODO
}


/**
 * @function waits until successfull pop from queue
 * @returns  pointer to popped element data
 */
void *TS_pop_from_queue_wait(ts_queue_t *queue) {
  // TODO
}


int main() {
  void *queue = TS_init_queue();
  TS_close_queue(queue);
  return 0;
}

