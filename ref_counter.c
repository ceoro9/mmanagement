#include <stdio.h>
#include <stdlib.h>


typedef void (*RC_destructor_t)(void*);


typedef struct  {
  void *data;
  RC_destructor_t dealloc;
} RC_payload_t;


typedef struct {
  uint ref_count;
  RC_payload_t payload;
} RC_object_t;


/**
 * @function RC_payload_t constructor
 */
RC_payload_t *init_RC_payload(void *data, RC_destructor_t dealloc) {

  RC_payload_t *result = (RC_payload_t*) malloc(sizeof(RC_payload_t));

  if (!result) {
    return NULL;
  }

  result->data = data;
  result->dealloc = dealloc;

  return result;
}


/**
 * @function RC_object_t constructor
 */
RC_object_t *init_RC_object(RC_payload_t *payload) {

  RC_object_t *result = (RC_object_t*) malloc(sizeof(RC_object_t));

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
void RC_dealloc(RC_object_t *obj) {
  RC_destructor_t dealloc = obj->payload.dealloc;
  (*dealloc)(obj->payload.data);
}


/**
 * @function increases number of references without NULL-check
 */
static inline void RC_incref(RC_object_t *obj) {
  obj->ref_count++;
}


/**
 * @function increases number of references WITH NULL-check
 */
static inline void RC_xincref(RC_object_t *obj) {
  if (obj != NULL) {
    RC_incref(obj);
  }
}


/**
 * @function decreases number of references without NULL-check
 */
static inline void RC_decref(RC_object_t *obj) {
  if (--obj->ref_count == 0) {
    RC_dealloc(obj);
  }
}


/**
 * @function decreases number of references WITH NULL-check
 */
static inline void RC_xdecref(RC_object_t *obj) {
  if (obj != NULL) {
    RC_decref(obj);
  }
}


int main() {


  return 0;
}
