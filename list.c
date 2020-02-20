#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#define LAMBDA(c_) ({ c_ _;})
#define DO_NOTHING_FUNC LAMBDA(void _(void*_) {})


/**
 * @function Initializes list's item data
 * @returns pointer to the created item data or NULL if error
 */
list_item_data_t *init_list_item_data(void *data, void(*free_func)(void*)) {

  list_item_data_t *result = (list_item_data_t*) malloc(sizeof(list_item_data_t));
  if (!result) {
    return NULL;
  }

  // default value
  if (!free_func) {
    free_func = DO_NOTHING_FUNC;
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

  if (!lid) {
    return;
  }

  // call provided callback to free data memory
  if (lid->free_func_ptr) {
    (*(lid->free_func_ptr))(lid->data_ptr);
  }

  // free item's memory itsels
  free(lid);
}


/**
 * @function Initializes list's item
 * @returns pointer to the newly created list's item
 */
list_item_t *init_list_item(list_item_data_t *data, list_item_t *next_item) {

  list_item_t *new_item = (list_item_t*) malloc(sizeof(list_item_t));

  new_item->data = data;
  new_item->next = next_item;

  return new_item;
}


/**
 * @function Lists' item destructor
 * @brief Calls item's data destructor and free item's memory itself
 */
void free_list_item(list_item_t *item) {

  if (!item) {
    return;
  }

  free_list_item_data(item->data);
  free(item);
}

/**
 * @function Initializes empty list
 * @returns pointer to the created list or NULL if error
 */
list_t* init_list() {

  // init dummy tail
  list_item_t* tail = init_list_item(NULL, NULL);
  if (!tail) {
    return NULL;
  }

  // init dummy head
  list_item_t* head = init_list_item(NULL, tail);
  if (!head) {
    free_list_item(tail);
    return NULL;
  }

  list_t *new_list = (list_t*) malloc(sizeof(list_t));
  if (!new_list) {
    free_list_item(tail);
    free_list_item(head);
    return NULL;
  }

  new_list->head      = head;
  new_list->tail      = tail;
  new_list->last_item = NULL;

  return new_list;
}


/**
 * @function Adds item's data to the head of list
 * @returns pointer to the created list's item
 */
list_item_t *add_item_to_head_of_list(list_t *list, list_item_data_t *item_data) {

  list_item_t *new_list_item = init_list_item(item_data, NULL);
  if (!new_list_item) {
    return NULL;
  }

  list_item_t *current_head_next_item = list->head->next;

  // if list is empty, new_list_item is last item in list
  if (current_head_next_item == list->tail) {
    list->last_item = new_list_item;
  }

  // Make head point to the new item
  // And new item to the head pointed to
  list->head->next = new_list_item;
  new_list_item->next = current_head_next_item;

  return new_list_item;
}


/**
 * @function Adds item's data to the tail of list
 * @returns pointer to the created list's item
 */
list_item_t *add_item_to_tail_of_list(list_t *list, list_item_data_t *item_data) {

  if (is_list_empty(list)) {
    return add_item_to_head_of_list(list, item_data);
  }

  list_item_t *new_list_item = init_list_item(item_data, NULL);
  if (!new_list_item) {
    return NULL;
  }

  list->last_item->next = new_list_item;
  new_list_item->next = list->tail;
  list->last_item = new_list_item;

  return new_list_item;
}


/**
 * @function Removes item from list
 * @returns 0 on success, -1 if item was not found
 */
int remove_item_from_list(list_t *list, list_item_t *searched_item) {

  list_item_t *prev_item = get_list_head(list);
  list_item_t *current_item = prev_item->next;

  while (current_item != get_list_tail(list) &&
         current_item != searched_item) {
    prev_item = current_item;
    current_item = current_item->next;
  }

  // If current item is tail - not found
  if (current_item == get_list_tail(list)) {
    return -1;
  }

  // if last item should be removed, change last_item pointer
  if (current_item->next == list->tail) {
    list->last_item = prev_item;
  }
  prev_item->next = current_item->next;
  free_list_item(current_item);

  return 0;
}


/**
 * @function removes first item from list
 * @returns pointer to item's data
 */
void *remove_and_get_item_data_from_head(list_t *list){

  if (is_list_empty(list)) {
    return NULL;
  }

  list_item_t *first_item = list->head->next;
  void *first_item_data = first_item->data->data_ptr;

  list->head->next = first_item->next;
  free_list_item(first_item);

  return first_item_data;
}


/**
 * @function removes last item from list
 * @returns pointer to item's data
 */
void *remove_and_get_item_data_from_tail(list_t *list) {

  if (is_list_empty(list)) {
    return NULL;
  }

  list_item_t *last_item = list->last_item;
  void *item_data = last_item->data->data_ptr;

  // TODO: get rid of linear complexity
  //       to find find prev to last item
  list_item_t *current_item = list->head;
  while (current_item->next != last_item) {
    current_item = current_item->next;
  }

  current_item->next = last_item->next;
  list->last_item = current_item == list->head ? NULL : current_item;
  free_list_item(last_item);

  return item_data;
}


/**
 * @function Cleans list
 * @returns number of deleted list items
 */
int clean_list(list_t *list) {

  int deleted_items_counter = 0;
  list_item_t *current_item = get_list_head(list)->next;
  list_item_t *end_item = get_list_tail(list);

  while (current_item != end_item) {

    list_item_t *next_item = current_item->next;
    free_list_item(current_item);

    current_item = next_item;
    ++deleted_items_counter;
  }

  get_list_head(list)->next = current_item;
  list->last_item = NULL;

  return deleted_items_counter;
}


/**
 * @function Cleans list's items and its memory itself
 */
void close_list(list_t *list) {

  clean_list(list);

  free_list_item(list->head);
  free_list_item(list->tail);
  free_list_item(list->last_item);

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

  // first elements becomes last one
  if (!is_list_empty(list)) {
    list->last_item = list->head->next;
  }

  list_item_t *last_item = reverse_list_inner(list->tail, list->head->next);
  last_item->next = list->head;

  // swap(list->head, list->tail)
  list_item_t *tmp = list->head;
  list->head = list->tail;
  list->tail = tmp;
}


/* int main() { */

/*   list_t *my_list = init_list(); */

/*   // init data in text memory of program, so this memory */
/*   // cann't be freed - we do nothing in clean up callback */
/*   list_item_data_t *lid_1, *lid_2, *lid_3, *lid_0; */
/*   lid_1 = init_list_item_data((void*) 1, DO_NOTHING_FUNC); */
/*   lid_2 = init_list_item_data((void*) 2, DO_NOTHING_FUNC); */
/*   lid_3 = init_list_item_data((void*) 3, DO_NOTHING_FUNC); */
/*   lid_0 = init_list_item_data((void*) 0, DO_NOTHING_FUNC); */

/*   // init data on heap, so in clean up callback */
/*   // we just pass pointer to free function from stdlib */
/*   int *hoh = (int*) malloc(sizeof(int)); */
/*   *hoh = 4; */
/*   list_item_data_t *lid_4 = init_list_item_data((void*) hoh, &free); */

/*   free_list_item_data(lid_4); */

/*   list_item_t *item_1 = add_item_to_head_of_list(my_list, lid_1); */
/*   list_item_t *item_2 = add_item_to_head_of_list(my_list, lid_2); */
/*   list_item_t *item_3 = add_item_to_head_of_list(my_list, lid_3); */
/*   list_item_t *item_0 = add_item_to_tail_of_list(my_list, lid_0); */

/*   printf("first element: %d\n", my_list->head->next->data->data_ptr); */
/*   printf("second element: %d\n", my_list->head->next->next->data->data_ptr); */
/*   printf("third element: %d\n", my_list->head->next->next->next->data->data_ptr); */
/*   printf("fourth element: %d\n", my_list->head->next->next->next->next->data->data_ptr); */

/*   reverse_list(my_list); */

/*   printf("\n===================\n\n"); */

/*   printf("first element: %d\n", my_list->head->next->data->data_ptr); */
/*   printf("second element: %d\n", my_list->head->next->next->data->data_ptr); */
/*   printf("third element: %d\n\n", my_list->head->next->next->next->data->data_ptr); */

/*   // */
/*   // */
/*   // */

/*   printf(is_list_empty(my_list) ? "list is empty\n" : "list is not empty\n"); */

/*   remove_item_from_list(my_list, item_1); */
/*   remove_item_from_list(my_list, item_2); */
/*   remove_item_from_list(my_list, item_3); */

/*   printf(is_list_empty(my_list) ? "list is empty\n" : "list is not empty\n"); */

/*   close_list(my_list); */

/*   return 0; */
/* } */

