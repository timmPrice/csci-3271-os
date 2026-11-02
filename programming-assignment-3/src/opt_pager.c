#include "pager.h"
#include "util/vector.h"
#include "util/checked_malloc.h"

typedef struct opt_pager_state {
    const vector* trace;
    int index;
} opt_pager_state;

int opt_translate(pager_interface* self, int virtual_address) {
    opt_pager_state* state = self->state;
    int physical = default_translate(self, virtual_address);
    state->index += 1;
    return physical;
}

int opt_handle_page_fault(pager_interface* self, int faulting_page) {
    opt_pager_state* state = self->state;
    
    int next = 0;
    int use_next = -1;

    for (int i = 0; i < self->common_state->frame_table->page_num; i++){
     
    }

    return 0;
}

int opt_pager_free(pager_interface* self) {
    if(self == NULL) {
        return -1;
    }
    common_state_free(self->common_state);
    free(self);
    return 0;
}

pager_interface* opt_pager_new(const memory_constants* memory_params, const vector* address_trace) {
    pager_interface* self = checked_malloc(sizeof(*self));
    opt_pager_state* state = checked_malloc(sizeof(*state));
    common_pager_state* common_state = common_state_new(memory_params);

    self->translate = opt_translate;
    self->handle_page_fault = opt_handle_page_fault;
    self->get_num_page_faults = default_get_num_page_faults;
    self->check_table_contains = default_check_table_contains;
    self->state = state;
    self->common_state = common_state;

    state->index = 0;
    state->trace = address_trace; 

    return self;
}
