#include "pager.h"
#include "util/checked_malloc.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct lru_pager_state{
    int accessed_counter;
    int* timestamp;
} lru_pager_state;

int lru_translate(pager_interface* self, int virtual_address) {
    lru_pager_state* state = self -> state;
    state->accessed_counter += 1;
    int physical = default_translate(self, virtual_address);
    int frame = physical / self->common_state->constants->page_size;
    state->timestamp[frame] = state->accessed_counter;
    return physical;
}

int lru_handle_page_fault(pager_interface* self, int faulting_page) {
    lru_pager_state* state = self -> state;
    int next = 0;

    for (int i = 1; i < self->common_state->constants->num_frames; i++){
        if (state->timestamp[i] < state->timestamp[next]){
            next = i;
        }
    }

    update_tables(self->common_state, faulting_page, next);
    return next;
}

pager_interface* lru_pager_new(const memory_constants* memory_params) {
    pager_interface* self = checked_malloc(sizeof(*self));
    lru_pager_state* state = checked_malloc(sizeof(*state));
    common_pager_state* common_state = common_state_new(memory_params);

    int state_size = memory_params->num_frames;
    int* timestamp = checked_malloc(sizeof(int) * state_size);
    
    for (int i = 0; i < state_size; i++) {
        timestamp[i] = 0;
    }
   
    state->accessed_counter = 0;
    state->timestamp = timestamp;
    self->translate = lru_translate;
    self->handle_page_fault = lru_handle_page_fault;
    self->get_num_page_faults = default_get_num_page_faults;
    self->check_table_contains = default_check_table_contains;
    self->state = state;
    self->common_state = common_state;
    return self;
}

int lru_pager_free(pager_interface* self) {
    if(self == NULL) {
        return -1;
    }
    lru_pager_state* a_state = self->state;
    free(a_state->timestamp);
    free(a_state);
    common_state_free(self->common_state);
    free(self);
    
    return 0;
}
