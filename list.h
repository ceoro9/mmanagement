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
typedef struct {
  list_item_t *head;
  list_item_t *tail;
  list_item_t *last_item; // item before tail
} list_t;


list_t* init_list();


list_item_data_t *init_list_item_data(void *data, void(*free_func)(void*));


static inline list_item_t *get_list_head(list_t *list) {
  return list->head;
}


static inline list_item_t *get_list_tail(list_t *list) {
  return list->tail;
}


static inline int is_list_empty(list_t *list) {
  return list->head->next == list->tail;
}


static inline void *get_list_item_data(list_item_t *list_item) {
  return list_item ? list_item->data->data_ptr : NULL;
}


static inline list_item_t *get_item_from_head_of_list(list_t *list) {
  return is_list_empty(list) ? NULL : list->head->next;
}


static inline list_item_t *get_item_from_tail_of_list(list_t *list) {
  return is_list_empty(list) ? NULL : list->last_item;
}


list_item_t *add_item_to_head_of_list(list_t *list, list_item_data_t *item_data);


list_item_t *add_item_to_tail_of_list(list_t *list, list_item_data_t *item_data);


int remove_item_from_list(list_t *list, list_item_t *searched_item);


void *remove_and_get_item_data_from_head(list_t *list);


void *remove_and_get_item_data_from_tail(list_t *list);


int clean_list(list_t *list);


void close_list(list_t *list);


void reverse_list(list_t *list);

