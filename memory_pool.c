#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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
} mpool_page_t;

/*
 * Memory pool list of allocated pages
*/
typedef struct mpool_pages_t {
    mpool_page_t page;
    struct mpool_pages_t *next_page;
} mpool_pages_t;

/*
 * Open/allocate new memory pool
*/
mpool_t *create_mpool(const unsigned int pool_size) {
    mpool_t *new_mpool_ptr = (mpool_t*) malloc(pool_size + sizeof(mpool_t));

    if (new_mpool_ptr == NULL) {
        return NULL;
    }

    new_mpool_ptr->size = pool_size;

    return new_mpool_ptr;
}

/*
 * Close/free memory pool
*/
void close_mpool(mpool_t *mp_ptr) {
    free(mp_ptr);
}

/*
 * Wipe memory pool
*/
mpool_t *clear_mpool(mpool_t *mp_ptr) {
    if (mp_ptr->pages != NULL) {
        errno = EACCES;
        return NULL;
    }

    mpool_t *new_mpool_t = (mpool_t*) realloc(mp_ptr, sizeof(mpool_t));

    if (new_mpool_t == NULL) {
        errno = ECANCELED;
        return NULL;
    }

    new_mpool_t->size = 0;

    return new_mpool_t;
}

int main() {
    mpool_t *mpool_ptr = create_mpool(1024);

    mpool_ptr = clear_mpool(mpool_ptr);

    close_mpool(mpool_ptr);

    return 0;
}