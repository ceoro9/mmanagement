#include <stdio.h>
#include <stdlib.h>
#define LAMBDA(c_) ({ c_ _;})
#define DO_NOTHING_FUNC LAMBDA(void _(void*_) {})


/**
 * @struct List's item data
 *
 * @prop {data_ptr} pointer to actual data
 * @prop {free_func_ptr} pointer to function to free holded memory
 */
typedef struct list_item_data_t_ {
  void *data_ptr;
  void (*free_func_ptr)(void*);
} list_item_data_t;


/**
 * @struct List's item
 *
 * @prop {data} item data
 * @prop {next} pointer to next list's item
 */
typedef struct list_item_t_ {
  list_item_data_t *data;
  struct list_item_t_ *next;
} list_item_t;


/**
 * @struct Linked list
 *
 * @prop {head} list's head(dummy list item)
 * @prop {tail} list's tail(dummy list item)
 */
typedef struct list_t_ {
  list_item_t *head;
  list_item_t *tail;
} list_t;


/**
 * @function Initializes list's item data
 * @returns pointer to the created item data or NULL if error
 */
list_item_data_t *init_list_item_data(void *data, void(*free_func)(void*)) {

  list_item_data_t *result = (list_item_data_t*) malloc(sizeof(list_item_data_t));
  if (!result) {
    return NULL;
  }

  result->data_ptr = data;
  result->free_func_ptr = free_func;

  return result;
}


/**
 * @function List's item data destructor
 * @brief Frees item's data memory by calling free_func_ptr
 *        callback and also frees item's memory itself
 */
void free_list_item_data(list_item_data_t *lid) {

  if (!lid || !lid->free_func_ptr) {
    return;
  }

  // call provided callback to free data memory
  (*(lid->free_func_ptr))(lid->data_ptr);

  // free item's memory itsels
  free(lid);
}


/**
 * @function Initializes empty list
 * @returns pointer to the created list or NULL if error
 */
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


/**
 * @function Adds item's data to the head of list
 * @returns pointer to the created list's item
 */
list_item_t *add_item_to_list(list_t *list, list_item_data_t *item_data) {

  list_item_t *new_list_item = malloc(sizeof(list_item_t));
  if (!new_list_item) {
    return NULL;
  }
  new_list_item->data = item_data;

  list_item_t *current_head_next_item = list->head->next;
  list->head->next = new_list_item;
  new_list_item->next = current_head_next_item;

  return new_list_item;
}


int is_list_empty(list_t *list) {
  return list->head->next == list->tail;
}


/**
 * @function Cleans list
 * @returns number of deleted list items
 */
int clean_list(list_t *list) {

  int deleted_items_counter = 0;
  list_item_t *current_item = list->head->next;
  list_item_t *end_item = list->tail;

  while (current_item != end_item) {

    list_item_t *next_item = current_item->next;

    // free holding data and list item itself
    free_list_item_data(current_item->data);
    free(current_item);

    current_item = next_item;
    ++deleted_items_counter;
  }

  list->head->next = current_item;

  return deleted_items_counter;
}


/**
 * @function Cleans list's items and its memory itself
 */
void close_list(list_t *list) {

  if (!is_list_empty(list)) {
    clean_list(list);
  }

  free(list->head);
  free(list->tail);
  free(list);
}

list_item_t *reverse_list_inner(list_item_t *tail, list_item_t *current) {

  if (current == tail) {
    return tail;
  }

  list_item_t *prev_to = reverse_list_inner(tail, current->next);
  prev_to->next = current;

  return current;
}

/**
 * @function Reverses list
 */
void reverse_list(list_t *list) {

  list_item_t *last_item = reverse_list_inner(list->tail, list->head->next);
  last_item->next = list->head;

  // swap(list->head, list->tail)
  list_item_t *tmp = list->head;
  list->head = list->tail;
  list->tail = tmp;
}


int main() {

  list_t *my_list = init_list();

  // init data in text memory of program, so this memory
  // cann't be freed - we do nothing in clean up callback
  list_item_data_t *lid_1, *lid_2, *lid_3;
  lid_1 = init_list_item_data((void*) 1, DO_NOTHING_FUNC);
  lid_2 = init_list_item_data((void*) 2, DO_NOTHING_FUNC);
  lid_3 = init_list_item_data((void*) 3, DO_NOTHING_FUNC);

  // init data on heap, so in clean up callback
  // we just pass pointer to free function from stdlib
  int *hoh = (int*) malloc(sizeof(int));
  *hoh = 4;
  list_item_data_t *lid_4 = init_list_item_data((void*) hoh, &free);

  free_list_item_data(lid_4);

  list_item_t *item_1 = add_item_to_list(my_list, lid_1);
  list_item_t *item_2 = add_item_to_list(my_list, lid_2);
  list_item_t *item_3 = add_item_to_list(my_list, lid_3);

  printf("First element: %d\n", my_list->head->next->data->data_ptr);
  printf("Second element: %d\n", my_list->head->next->next->data->data_ptr);
  printf("Third element: %d\n", my_list->head->next->next->next->data->data_ptr);

  reverse_list(my_list);

  printf("\n===================\n\n");

  printf("First element: %d\n", my_list->head->next->data->data_ptr);
  printf("Second element: %d\n", my_list->head->next->next->data->data_ptr);
  printf("Third element: %d\n\n", my_list->head->next->next->next->data->data_ptr);

  //
  //
  //

  printf(is_list_empty(my_list) ? "list is empty\n" : "list is not empty\n");

  clean_list(my_list);

  printf(is_list_empty(my_list) ? "list is empty\n" : "list is not empty\n");

  close_list(my_list);

  return 0;
}

