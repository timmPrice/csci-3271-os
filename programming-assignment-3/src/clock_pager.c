#include "pager.h"
#include "util/checked_malloc.h"

#include <stdbool.h>
#include <stdio.h>

typedef struct clock_pager_state {
    int hand;
    bool* reference_bits;
} clock_pager_state ;

int clock_translate(pager_interface* self, int virtual_address){
    clock_pager_state* state = self->state;
    int physical = default_translate(self, virtual_address);
    int frame = physical / self->common_state->constants->page_size;
    state->reference_bits[frame] = true;
    return physical;
}

int clock_handle_page_fault(pager_interface* self, int faulting_page) {
    clock_pager_state* state = self->state;
    int next = 0;

    while (state->reference_bits[state->hand] == true) {
        state->reference_bits[state->hand] = false;
        state->hand = (state->hand+1) % self->common_state->constants->num_frames;
    }
    
    next = state->hand;
    update_tables(self->common_state, faulting_page, next);
    state->hand = (state->hand+1) % self->common_state->constants->num_frames;
    return next;
}

pager_interface* clock_pager_new(const memory_constants* memory_params) {
    pager_interface* self = checked_malloc(sizeof(*self));
    clock_pager_state* state = checked_malloc(sizeof(*state));
    common_pager_state* common_state = common_state_new(memory_params);

    self->translate = clock_translate;
    self->handle_page_fault = clock_handle_page_fault;
    self->get_num_page_faults = default_get_num_page_faults;
    self->check_table_contains = default_check_table_contains;
    self->state = state;
    self->common_state = common_state;

    state->hand = 0;
    state->reference_bits = checked_malloc(sizeof(bool) * memory_params->num_frames);

    for (int i = 0; i < memory_params->num_frames; i++){
        state->reference_bits[i] = false;
    }

    return self;
}

int clock_pager_free(pager_interface* self) {
    if (self == NULL) {
        return -1;
    }
    clock_pager_state* a_state = self->state;
    free(a_state->reference_bits);
    free(a_state);
    common_state_free(self->common_state);
    free(self);
    return 0;
}
