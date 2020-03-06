typedef struct {

  // List with sequance of entities
  ts_list_t *list;

} ts_queue_t;


ts_queue_t *TS_init_queue();


void TS_close_queue(ts_queue_t *queue);


void TS_push_to_queue(ts_queue_t *queue, void *data);


void *TS_pop_from_queue(ts_queue_t *queue);


void *TS_pop_from_queue_wait(ts_queue_t *queue);

