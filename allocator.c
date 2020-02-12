#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * @struct meta info about allocated block
 */
typedef struct {
  unsigned int size    : 31; // 31 bits to store size of block
  unsigned int is_free : 1;  // 1 bit to store if block is free
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

// proc_break holds pointer to the process break
static void *proc_break = NULL;

// FIRST_ALLOCATION_SIZE holds value of the first allocation in bytes
static const int FIRST_ALLOCATION_SIZE = 50;

// min_block_size ...
static const int min_block_size = 8;

// get_metadata_size Metadata size selector
static inline int get_metadata_size() {
  return sizeof(RC_block_t);
}


/**
 * @function inits allocator
 */
void RC_init() {

  if (!heap_start) {
    int alloc_size = FIRST_ALLOCATION_SIZE + get_metadata_size();

    heap_start = sbrk(alloc_size);
    proc_break = ((char*) heap_start) + alloc_size;

    RC_block_t *new_block = (RC_block_t*) heap_start;

    new_block->info.size = FIRST_ALLOCATION_SIZE;
    new_block->info.is_free = 1;
  }

  return;
}


/**
 * @function mallocs_memory with requested size
 */
void *RC_malloc(size_t requested_size) {

  if (!heap_start || !requested_size) {
    return NULL;
  }

  RC_block_t *current_block = (RC_block_t*) heap_start;

  while (current_block != proc_break) {

    // found free block with sufficient size
    if (current_block->info.is_free &&
        current_block->info.size >= requested_size) {

      int size_diff = current_block->info.size - requested_size;
      int metadata_size = get_metadata_size();

      current_block->info.is_free = 0;

      printf("size_diff = %d, proc_break = %p ", size_diff, proc_break);

      // split one block on two
      if (size_diff >= min_block_size + metadata_size) {

        RC_block_t *new_block = (RC_block_t*) (current_block->data + requested_size);

        printf("current_block = %p, new_block = %p", current_block, new_block);

        new_block->info.is_free = 1;
        new_block->info.size = size_diff - metadata_size;

        current_block->info.size = requested_size;
      }

      return (void*) current_block->data;
    }

    current_block = (RC_block_t*) (current_block->data + current_block->info.size);
  }

  // no more memory -> allocate new block
  RC_block_t *new_block = (RC_block_t*) sbrk(requested_size);

  // unsufficent request to obtain more memory
  if (!new_block) {
    return NULL;
  }

  proc_break = ((char*) new_block) + requested_size;

  new_block->info.is_free = 0;
  new_block->info.size = requested_size;

  return new_block->data;
}


/**
 * @function frees allocated memory
 */
void RC_free(void *data) {

  int metadata_size = get_metadata_size();

  RC_block_t *current_block = (RC_block_t*) ((char*) data - metadata_size);
  RC_block_t *next_block    = (RC_block_t*) ((char*) current_block->data + current_block->info.size);

  current_block->info.is_free = 1;

  if (next_block == proc_break) {
    return;
  }

  // if next block is free - coalesce with the current one
  if (next_block->info.is_free) {
    current_block->info.size += next_block->info.size + metadata_size;
  }

  return;
}


void print_allocated_blocks() {

  int metadata_size = get_metadata_size();
  RC_block_t *current_block = (RC_block_t*) heap_start;

  printf("Process break: %p, ", proc_break);
  printf("Allocated blocks: ");

  while (current_block != proc_break) {
    printf(
           "(size=%d, is_free=%d, start=%p, end=%p) ",
           current_block->info.size,
           current_block->info.is_free,
           current_block,
           ((char*) current_block->data) + current_block->info.size - 1
           );
    current_block = (void*) (current_block->data + current_block->info.size);
  }

  printf("\n");

  return;
}


void print_buffer(void *input_buf, int size) {

  char *buffer = (char*) input_buf;

  printf("Buffer: starts on %p, ends on %p. Bytes: ", (void*) buffer, (void*) (buffer + size));

  for (int i = 0; i < size; ++i) {
    printf("%x ", *(buffer + i));
  }

  printf("\n");

  return;
}

int main() {

  RC_init();
  void *data = RC_malloc(10);

  printf("\n\n");
  print_allocated_blocks();

  RC_free(data);
  printf("\n\n");
  print_allocated_blocks();

  /* int buffer_size = 10; */
  /* char *buffer = (char*) RC_malloc(buffer_size); */
  /* memset(buffer, 0x0, buffer_size); */

  /* // print_buffer(buffer, buffer_size); */

  /* int new_buffer_size = 30; */
  /* char *new_buffer = (char*) RC_malloc(new_buffer_size); */

  /* memset(new_buffer, 0xFF, new_buffer_size); */

  /* print_buffer(new_buffer, new_buffer_size); */

  /* printf("\n%p\n", proc_break); */

  /* print_allocated_blocks(); */

  /* RC_free(new_buffer); */
  /* RC_free(buffer); */

  /* RC_block_t *first_block = (RC_block_t*) ((char*) buffer - get_metadata_size()); */

  /* printf("%d", first_block->info.size); */

  return 0;
}
