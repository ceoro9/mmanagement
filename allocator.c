#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define ALLOCATED_BLOCK_FLAG 0
#define FREE_BLOCK_FLAG 1

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
  RC_block_meta_t prev_block_info;
  RC_block_meta_t current_block_info;
  char data[0];
} RC_block_t;


// heap_start holds pointer to the start of heap
static void *heap_start = NULL;

// proc_break holds pointer to the process break
static void *proc_break = NULL;

// last_block holds pointer to the last allocated block
static RC_block_t *RC_last_block = NULL;

// FIRST_ALLOCATION_SIZE holds value of the first allocation in bytes
static const int FIRST_ALLOCATION_SIZE = 50;

// min_block_size ...
static const int min_block_size = 8;

// get_metadata_size Metadata size selector
static inline int get_metadata_size() {
  return sizeof(RC_block_meta_t) * 2;
}

// set_last_block ...
void set_last_block(RC_block_t *new_last_block) {
  RC_last_block = new_last_block;
}

// is_first_block checks if provided block is first in virtual memory heap
static inline int is_first_block(RC_block_t *block) {
  return block == heap_start;
}

// is_last_block checks if provided block is last in virtual memory heap
static inline int is_last_block(RC_block_t *block) {
  return block == RC_last_block;
}

/**
 * @function sets current block metadata
 */
void set_current_block_metadata(RC_block_t *block, unsigned int size, unsigned int is_free) {

  if (!block) {
    return;
  }

  block->current_block_info.size = size;
  block->current_block_info.is_free = is_free;

  return;
}

/**
 * @function sets prev block metadata
 */
void set_prev_block_metadata(RC_block_t *block, unsigned int size, unsigned int is_free) {

  if (!block) {
    return;
  }

  block->prev_block_info.size = size;
  block->prev_block_info.is_free = is_free;

  return;
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
    RC_last_block = new_block;

    // init first block metadata
    set_prev_block_metadata(new_block, -1, FREE_BLOCK_FLAG);
    set_current_block_metadata(new_block, FIRST_ALLOCATION_SIZE, FREE_BLOCK_FLAG);
  }

  return 0;
}


/**
 * @function calls sbrk to allocate new virtual memory
 */
void *allocate_new_block(size_t requested_size) {

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

  // TODO: separate function to handle first block metadata
  if (!RC_last_block) {
    set_prev_block_metadata(new_block, -1, FREE_BLOCK_FLAG);
  } else {
    set_prev_block_metadata(
                            new_block,
                            RC_last_block->current_block_info.size,
                            RC_last_block->current_block_info.is_free
                            );
  }

  set_current_block_metadata(new_block, requested_size, ALLOCATED_BLOCK_FLAG);
  set_last_block(new_block);

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
    return allocate_new_block(requested_size);
  }

  int metadata_size = get_metadata_size();
  RC_block_t *source_block = (RC_block_t*) heap_start;

  // while source block is not last one
  while (source_block != proc_break) {

    // found free block with sufficient size
    if (source_block->current_block_info.is_free &&
        source_block->current_block_info.size >= requested_size) {

      int size_diff = source_block->current_block_info.size - requested_size;

      source_block->current_block_info.is_free = ALLOCATED_BLOCK_FLAG;

      // split one block on two
      if (size_diff >= min_block_size + metadata_size) {

        source_block->current_block_info.size = requested_size;

        RC_block_t *new_block = (RC_block_t*) (source_block->data + requested_size);

        // set new_block metadata
        set_current_block_metadata(
                                   new_block,
                                   size_diff - metadata_size,
                                   FREE_BLOCK_FLAG
                                   );
        set_prev_block_metadata(
                                new_block,
                                source_block->current_block_info.size,
                                source_block->current_block_info.is_free
                                );
      }

      return (void*) source_block->data;
    }

    source_block = (RC_block_t*) (source_block->data + source_block->current_block_info.size);
  }

  // else if block with sufficient size was not found -> allocate new one
  return allocate_new_block(requested_size);
}


/**
 * @function frees allocated memory
 */
void RC_free(void *data) {

  int metadata_size = get_metadata_size();
  RC_block_t *source_block = (RC_block_t*) ((char*) data - metadata_size);

  // whatever happens next, source block should be free
  source_block->current_block_info.is_free = FREE_BLOCK_FLAG;

  //
  // try to coalesce with next block if source
  // block is not last and next block is free
  if (!is_last_block(source_block)){

    RC_block_t *next_block = (RC_block_t*) ((char*) source_block->data + source_block->current_block_info.size);

    if (next_block->current_block_info.is_free) {
      source_block->current_block_info.size += next_block->current_block_info.size + metadata_size;
    } else {
      next_block->prev_block_info.is_free = FREE_BLOCK_FLAG;
    }
  }

  //
  // try to coalescce with prev block if source
  // block is not first and prev block is free
  if (!is_first_block(source_block) && source_block->prev_block_info.is_free) {

    RC_block_t *prev_block = (RC_block_t*) (((char*) source_block) - (source_block->prev_block_info.size + metadata_size));

    prev_block->current_block_info.size += source_block->current_block_info.size + metadata_size;
    prev_block->prev_block_info = source_block->prev_block_info;
  }

  return;
}


void print_allocated_blocks() {

  int metadata_size = get_metadata_size();
  RC_block_t *source_block = (RC_block_t*) heap_start;

  printf("Process break: %p, ", proc_break);
  printf("Allocated blocks: ");

  while (source_block != proc_break) {
    printf(
           "(size=%d, is_free=%d, start=%p, end=%p) ",
           source_block->current_block_info.size,
           source_block->current_block_info.is_free,
           source_block,
           ((char*) source_block->data) + source_block->current_block_info.size - 1
           );
    source_block = (void*) (source_block->data + source_block->current_block_info.size);
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

  RC_free(data_1);
  RC_free(data_3);
  RC_free(data_2);

  printf("\n\n");
  print_allocated_blocks();

  return 0;
}

