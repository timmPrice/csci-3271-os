#include "pager.h"
#include "util/checked_malloc.h"

#include <stdlib.h>
#include <time.h>

int random_handle_page_fault(pager_interface* self, int faulting_page) {
    // Pick a random number between 0 and num_frames - 1
    int victim_frame = rand() / ((RAND_MAX + 1u) / (self->common_state->constants->num_frames - 1));
    update_tables(self->common_state, faulting_page, victim_frame);
    return victim_frame;
}

pager_interface* random_pager_new(const memory_constants* memory_params) {
    pager_interface* self = checked_malloc(sizeof(*self));
    common_pager_state* common_state = common_state_new(memory_params);
    // Seed the random number generator
    srand(time(NULL));

    self->translate = default_translate;
    self->handle_page_fault = random_handle_page_fault;
    self->check_table_contains = default_check_table_contains;
    self->get_num_page_faults = default_get_num_page_faults;
    // This pager doesn't need any of its own state
    self->state = NULL;
    self->common_state = common_state;
    return self;
}

int random_pager_free(pager_interface* self) {
    if (self == NULL) {
        return -1;
    }
    common_state_free(self->common_state);
    free(self);
    return 0;
}