#include "pager.h"
#include "util/checked_malloc.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct fifo_pager_state {
    int next_frame;
} fifo_pager_state;

int fifo_handle_page_fault(pager_interface* self, int faulting_page) {
    fifo_pager_state* state = self -> state;
    int next = state->next_frame;
    update_tables(self -> common_state, faulting_page, next);
    state -> next_frame = (next + 1) % self->common_state->constants->num_frames;
    return next;
}

pager_interface* fifo_pager_new(const memory_constants* memory_params) {
    fifo_pager_state* state = checked_malloc(sizeof(*state));
    state->next_frame = 0;
    pager_interface* self = checked_malloc(sizeof(*self));
    common_pager_state* common_state = common_state_new(memory_params);

    self->translate = default_translate;
    self->handle_page_fault = fifo_handle_page_fault;
    self->get_num_page_faults = default_get_num_page_faults;
    self->check_table_contains = default_check_table_contains;
    self->state = state;
    self->common_state = common_state;
    return self;
}

int fifo_pager_free(pager_interface* self) {
    if(self == NULL) {
        return -1;
    }
    common_state_free(self->common_state);
    free(self->state);
    free(self);
    return 0;
}
