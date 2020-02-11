#include <stdio.h>
#include <stdlib.h>


typedef void (*destructor_t)(void*);


typedef struct  {
  void *data;
  destructor_t dealloc;
} ref_counter_payload_t;


typedef struct {
  uint ref_count;
  ref_counter_payload_t payload;
} ref_counter_meta_t;


/**
 * @function ref_counter_payload_t constructor
 */
ref_counter_payload_t *init_ref_counter_payload(void *data, destructor_t dealloc) {

  ref_counter_payload_t *result = (ref_counter_payload_t*) malloc(sizeof(ref_counter_payload_t));

  if (!result) {
    return NULL;
  }

  result->data = data;
  result->dealloc = dealloc;

  return result;
}


/**
 * @function ref_counter_meta_t constructor
 */
ref_counter_meta_t *init_ref_counter_meta(ref_counter_payload_t *payload) {

  ref_counter_meta_t *result = (ref_counter_meta_t*) malloc(sizeof(ref_counter_meta_t));

  if (!result) {
    return NULL;
  }

  result->ref_count = 1;
  result->payload = *payload;

  return result;
}


/**
 * Deallocs holded memory
 */
void RC_dealloc(ref_counter_meta_t *op) {
  destructor_t dealloc = op->payload.dealloc;
  (*dealloc)(op->payload.data);
}


/**
 * @function increases number of references to data without NULL-check
 */
static inline void RC_incref(ref_counter_meta_t *op) {
  op->ref_count++;
}


/**
 * @function increases number of references to data WITH NULL-check
 */
static inline void RC_xincref(ref_counter_meta_t *op) {
  if (op != NULL) {
    RC_incref(op);
  }
}


/**
 * @function decreases number of references to data without NULL-check
 */
static inline void RC_decref(ref_counter_meta_t *op) {
  if (--op->ref_count == 0) {
    RC_dealloc(op);
  }
}


/**
 * @function decreases number of references to data WITH NULL-check
 */
static inline void Rc_xdecref(ref_counter_meta_t *op) {
  if (op != NULL) {
    RC_decref(op);
  }
}


int main() {


  return 0;
}
