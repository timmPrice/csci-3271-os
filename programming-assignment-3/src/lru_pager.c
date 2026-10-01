#include "pager.h"
#include "util/checked_malloc.h"

#include <stdlib.h>
#include <stdio.h>

int lru_handle_page_fault(pager_interface* self, int faulting_page) {
    //TODO: Implement this method

    return 0;
}

pager_interface* lru_pager_new(const memory_constants* memory_params) {
    pager_interface* self = checked_malloc(sizeof(*self));
    common_pager_state* common_state = common_state_new(memory_params);

    self->translate = default_translate;
    self->handle_page_fault = lru_handle_page_fault;
    self->get_num_page_faults = default_get_num_page_faults;
    self->check_table_contains = default_check_table_contains;
    self->state = NULL;
    self->common_state = common_state;
    return self;
}

int lru_pager_free(pager_interface* self) {
    if(self == NULL) {
        return -1;
    }
    common_state_free(self->common_state);
    free(self);
    return 0;
}
