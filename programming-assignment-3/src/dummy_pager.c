#include "pager.h"
#include "util/checked_malloc.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct dummy_pager_state {
    int access_count;
} dummy_pager_state;

int dummy_translate(pager_interface* self, int virtual_address) {
    dummy_pager_state* my_state = self->state;
    my_state->access_count++;
    return default_translate(self, virtual_address);
}

int dummy_handle_page_fault(pager_interface* self, int faulting_page) {
    int free_frame = -1;
    for(int frame = 0; frame < self->common_state->constants->num_frames; frame++) {
        if(!self->common_state->frame_table[frame].in_use) {
            free_frame = frame;
            break;
        }
    }
    if(free_frame == -1) {
        fprintf(stderr, "Dummy pager ran out of free memory!");
        exit(EXIT_FAILURE);
    }
    update_tables(self->common_state, faulting_page, free_frame);
    return free_frame;
}


pager_interface* dummy_pager_new(const memory_constants* memory_params) {
    pager_interface* self = checked_malloc(sizeof(*self));
    dummy_pager_state* state = checked_malloc(sizeof(*state));
    common_pager_state* common_state = common_state_new(memory_params);

    self->translate = dummy_translate;
    self->handle_page_fault = dummy_handle_page_fault;
    self->get_num_page_faults = default_get_num_page_faults;
    self->check_table_contains = default_check_table_contains;
    self->common_state = common_state;
    self->state = state;
    return self;
}

int dummy_pager_free(pager_interface* self) {
    if(self == NULL) {
        return -1;
    }
    common_state_free(self->common_state);
    free(self->state);
    free(self);
    return 0;
}
