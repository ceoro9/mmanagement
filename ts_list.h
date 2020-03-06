typedef struct {

  pthread_mutex_t add_head_mutex;
  pthread_mutex_t add_tail_mutex;
  pthread_mutex_t remove_mutex;

  list_t *origin_list;

} ts_list_t;


ts_list_t *init_ts_list();


void TS_close_list(ts_list_t *list);


list_item_t *TS_add_item_to_head_of_list(ts_list_t *list, list_item_data_t *item_data);


list_item_t *TS_add_item_to_tail_of_list(ts_list_t *list, list_item_data_t *item_data);


int TS_remove_item_from_list(ts_list_t *list, list_item_t *searched_item);


