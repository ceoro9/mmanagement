#include <stdio.h>
#include <stdlib.h>

typedef struct list_item_t_ {
  void* data;
  struct list_item_t_ *next;
} list_item_t;

typedef struct list_t_ {
  list_item_t *head;
  list_item_t *tail;
} list_t;

list_t* init_list() {

  // init dummy head
  list_item_t* head = (list_item_t*) malloc(sizeof(list_item_t));
  if (!head) {
    return NULL;
  }

  // init dummy tail
  list_item_t* tail = (list_item_t*) malloc(sizeof(list_item_t));
  if (!tail) {
    free(head);
    return NULL;
  }

  head->data = NULL;
  tail->data = NULL;

  head->next = tail;
  tail->next = NULL;

  list_t *result_list = malloc(sizeof(list_t));
  if (!result_list) {
    free(head);
    free(tail);
    return NULL;
  }

  result_list->head = head;
  result_list->tail = tail;

  return result_list;
}

list_item_t *get_list_head(list_t *list) {
  return list->head;
}

list_item_t *get_list_tail(list_t *list) {
  return list->tail;
}

list_item_t *add_item_to_list(list_t *list, void *data) {

  list_item_t *new_list_item = malloc(sizeof(list_item_t));
  if (!new_list_item) {
    return NULL;
  }
  new_list_item->data = data;

  list_item_t *current_head_next_item = list->head->next;
  list->head->next = new_list_item;
  new_list_item->next = current_head_next_item;

  return new_list_item;
}

int is_list_empty(list_t *list) {
  return list->head->next == list->tail;
}

/**
 * Cleans list
 * @returns number of deleted list items
 */
int clean_list(list_t *list) {

  int deleted_items_counter = 0;
  list_item_t *current_item = list->head->next;
  list_item_t *end_item = list->tail;

  while (current_item != end_item) {
    list_item_t *next_item = current_item->next;
    free(current_item);
    current_item = next_item;
    ++deleted_items_counter;
  }

  list->head->next = current_item;

  return deleted_items_counter;
}

void close_list(list_t *list) {

  if (!is_list_empty(list)) {
    clean_list(list);
  }

  free(list->head);
  free(list->tail);
  free(list);
}

int main() {

  list_t *my_list = init_list();

  list_item_t *item_1 = add_item_to_list(my_list, (void*) 1);
  list_item_t *item_2 = add_item_to_list(my_list, (void*) 2);
  list_item_t *item_3 = add_item_to_list(my_list, (void*) 3);

  printf("%d\n", item_1->data);
  printf("%d\n", item_2->data);
  printf("%d\n", item_3->data);

  printf(is_list_empty(my_list) ? "List is empty\n" : "List is NOT empty\n");

  clean_list(my_list);

  printf(is_list_empty(my_list) ? "List is empty\n" : "List is NOT empty\n");

  close_list(my_list);

  return 0;
}
