#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define MPOOL_DEFAUL_PAGE_SIZE 64

extern int errno;

/*
 * Memory pool
*/
typedef struct mpool_t {
    unsigned int size;
    struct mpool_pages_t *pages;
} mpool_t;

/*
 * Memory pool allocated page
*/
typedef struct mpool_page_t {
    char *start_addr;
    unsigned int size;
    int is_allocated : 1;
} mpool_page_t;

/*
 * Memory pool list of allocated pages
*/
typedef struct mpool_pages_t {
    mpool_page_t page;
    struct mpool_pages_t *next_page;
} mpool_pages_t;

/*
 * Allocate memory for n pool pages 
*/
mpool_pages_t *create_mpool_n_pages(const unsigned int number_of_pages, char *start_address) {
    mpool_pages_t *result_mpool_page_ptr = (mpool_pages_t*) malloc(number_of_pages * sizeof(mpool_pages_t));

    if (result_mpool_page_ptr == NULL) {
        return NULL;
    }

    mpool_pages_t *current_mpool_page_ptr = result_mpool_page_ptr;

    // set mpool_pages pointers to the next page
    for (int n = 1; n < number_of_pages; ++n) {
        current_mpool_page_ptr->next_page = current_mpool_page_ptr + 1;
        current_mpool_page_ptr += 1;
    }

    current_mpool_page_ptr = result_mpool_page_ptr;

    // init mpool_pages
    for (int n = 0; n < number_of_pages; ++n) {
        current_mpool_page_ptr->page.is_allocated = 0;
        current_mpool_page_ptr->page.size = MPOOL_DEFAUL_PAGE_SIZE;
        current_mpool_page_ptr->page.start_addr = start_address + n * MPOOL_DEFAUL_PAGE_SIZE;
        current_mpool_page_ptr += 1;
    }

    return result_mpool_page_ptr;
}

/*
 * Open/allocate new memory pool
*/
mpool_t *create_mpool(const unsigned int pool_size) {
    const int mpool_page_size = MPOOL_DEFAUL_PAGE_SIZE;
    const int number_of_mpool_pages = (int) ceil((double) pool_size / mpool_page_size);

    mpool_t *new_mpool_ptr = (mpool_t*) malloc(sizeof(mpool_t) + number_of_mpool_pages * mpool_page_size);

    if (new_mpool_ptr == NULL) {
        return NULL;
    }
    
    mpool_pages_t *mpool_pages_ptr = create_mpool_n_pages(number_of_mpool_pages, (char*) new_mpool_ptr + sizeof(mpool_t));
    
    if (mpool_pages_ptr == NULL) {
        free(new_mpool_ptr);
        return NULL;
    }

    new_mpool_ptr->size = pool_size;
    new_mpool_ptr->pages = mpool_pages_ptr;

    return new_mpool_ptr;
}

/*
 * Free memory of all pool pages
*/
void free_mpool_pages(mpool_pages_t *mpool_pages_ptr) {
    free(mpool_pages_ptr);
}

/*
 * Close/free memory pool
*/
void close_mpool(mpool_t *mp_ptr) {
    free_mpool_pages(mp_ptr->pages);
    free(mp_ptr);
}

/*
 * Wipe memory pool
*/
mpool_t *clear_mpool(mpool_t *mp_ptr) {
    mpool_t *new_mpool_t = (mpool_t*) realloc(mp_ptr, sizeof(mpool_t));

    if (new_mpool_t == NULL) {
        errno = ECANCELED;
        return NULL;
    }

    free_mpool_pages(new_mpool_t->pages);

    new_mpool_t->size = 0;
    new_mpool_t->pages = NULL;

    return new_mpool_t;
}

int main() {
    mpool_t *mpool_ptr = create_mpool(1024);

    if (mpool_ptr == NULL) {
        perror("create_mpool: ");
        return 1;
    }

    mpool_ptr = clear_mpool(mpool_ptr);

    if (mpool_ptr == NULL) {
        perror("clear_mpool: ");
        return 1;
    }

    close_mpool(mpool_ptr);

    return 0;
}