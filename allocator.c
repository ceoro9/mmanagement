#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/**
 * @struct meta info about allocated block
 */
typedef struct {
  unsigned int block_size : 31; // 31 bits to store size of block
  unsigned int is_free    : 1;  // 1 bit to store if block is free
} RC_block_meta_t;


/**
 * @struct allocated block structure
 */
typedef struct {
  RC_block_meta_t info;
  char data[0];
} RC_block_t;


// heap_start holds pointer to the start of heap
static void *heap_start = NULL;

// FIRST_ALLOCATION_SIZE holds value of the first allocation in bytes
static const int FIRST_ALLOCATION_SIZE = 1024;


/**
 * @function inits allocator
 */
void RC_init() {
  if (!heap_start) {
    heap_start = sbrk(FIRST_ALLOCATION_SIZE);

    RC_block_t *p = (RC_block_t*) heap_start;
    int metadata_size = sizeof(RC_block_t);

    p->info.block_size = FIRST_ALLOCATION_SIZE - metadata_size;
    p->info.is_free = 1;
  }
}


/**
 * @function mallocs_memory
 */
void *RC_malloc(size_t size) {

  if (!heap_start) {
    return NULL;
  }

  RC_block_t *p = (RC_block_t*) heap_start;

  if (p->info.is_free) {
    p->info.is_free = 0;
    return p->data;
  }

  // TODO

  return NULL;
}


/**
 * @function frees allocated memory
 */
void RC_free(void *data) {
  RC_block_t *p = (RC_block_t*) ((char*) data - sizeof(RC_block_t)); 
  p->info.is_free = 1;
  // TODO: decrease fragmentation
}

int main() {

  RC_init();
  void *data = RC_malloc(10);

  printf("%p", data);

  RC_free(data);
  
  getchar();
  
  return 0;
}
