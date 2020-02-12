#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


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
 * @function inits allocator (optional to call)
 */
int RC_init() {

  if (!heap_start) {

    int alloc_size = FIRST_ALLOCATION_SIZE + get_metadata_size();

    heap_start = sbrk(alloc_size);

    // unable to allocated memory
    if (*((int*) heap_start) == -1) {
      return -1;
    }

    proc_break = sbrk(0);

    RC_block_t *new_block = (RC_block_t*) heap_start;

    new_block->info.size = FIRST_ALLOCATION_SIZE;
    new_block->info.is_free = 1;
  }

  return 0;
}


/**
 * @function calls sbrk to allocate new virtual memory
 */
void *allocate_new_memory(size_t requested_size) {

  int metadata_size = get_metadata_size();

  // no more memory -> allocate new block
  int alloc_size = requested_size + metadata_size;
  void *new_proc_break = sbrk(alloc_size); // new_proc_break == proc_break

  // unsufficent request to obtain more memory
  if (*((int*) new_proc_break) == -1) {
    return NULL;
  }

  // if RC_init was no called
  if (!heap_start) {
    heap_start = new_proc_break;
  }

  RC_block_t *new_block = (RC_block_t*) new_proc_break;
  proc_break = (void*) (((char*) new_proc_break) + alloc_size);

  new_block->info.is_free = 0;
  new_block->info.size = requested_size;

  return new_block->data;
}


/**
 * @function mallocs_memory with requested size
 */
void *RC_malloc(size_t requested_size) {

  if (!requested_size) {
    return NULL;
  }

  if (!heap_start) {
    return allocate_new_memory(requested_size);
  }

  int metadata_size = get_metadata_size();
  RC_block_t *current_block = (RC_block_t*) heap_start;

  while (current_block != proc_break) {

    // found free block with sufficient size
    if (current_block->info.is_free &&
        current_block->info.size >= requested_size) {

      int size_diff = current_block->info.size - requested_size;

      current_block->info.is_free = 0;

      // split one block on two
      if (size_diff >= min_block_size + metadata_size) {

        RC_block_t *new_block = (RC_block_t*) (current_block->data + requested_size);

        new_block->info.is_free = 1;
        new_block->info.size = size_diff - metadata_size;

        current_block->info.size = requested_size;
      }

      return (void*) current_block->data;
    }

    current_block = (RC_block_t*) (current_block->data + current_block->info.size);
  }

  // else if block with sufficient size was not found
  return allocate_new_memory(requested_size);
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

  void *data_1 = RC_malloc(10);
  void *data_2 = RC_malloc(100);
  void *data_3 = RC_malloc(500);

  printf("\n\n");
  print_allocated_blocks();

  return 0;
}

