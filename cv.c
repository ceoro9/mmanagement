#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct cv_node_ {
    pthread_mutex_t *dynamic;
    bool is_awaken;
    struct cv_node_ *next;
} cv_node;

typedef struct {
    cv_node *head;
} cond_t;

cond_t *cond_init() {
    
    cond_t *result_cond = (cond_t*) malloc(sizeof(cond_t));

    result_cond->head = NULL;

    return result_cond;
}

void cond_destroy(cond_t *cv) {
    // TODO: destroy all cv nodes may be
    free(cv);
}

/*
 * Function assumes mutex is locked
 * 
 * Error codes:
 * 0 - success
 * 1 - cv_node is not found
 * 2 - invalid input
*/
static int remove_from_linked_list(cond_t *cv, cv_node *target_cv_node_ptr) {

    // sanity checking
    if (target_cv_node_ptr == NULL) {
        return 2;
    }

    if (target_cv_node_ptr == cv->head) {
        cv->head = cv->head->next;
        return 0;
    }

    for (
        cv_node *prev_cv_node_ptr;
        prev_cv_node_ptr->next != NULL;
        prev_cv_node_ptr = prev_cv_node_ptr->next) {

            if (prev_cv_node_ptr->next == target_cv_node_ptr) {
                prev_cv_node_ptr->next = prev_cv_node_ptr->next->next;
                return 0;
            }
    }

    return 1;
}

// TODO
void cond_wait(cond_t *cv, pthread_mutex_t *m) {

}

// TODO
void cond_signal(cond_t *cv) {

}

// TODO
void cond_broadcast(cond_t *cv) {

}

int main() {

    cond_t *cv = cond_init();

    cond_destroy(cv);

}