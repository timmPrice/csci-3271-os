#include "pager.h"
#include "util/checked_malloc.h"

common_pager_state* common_state_new(const memory_constants* memory_params) {
    common_pager_state* state = checked_malloc(sizeof(*state));
    state->frame_table = checked_malloc(memory_params->num_frames * sizeof(frame_table_entry));
    for(int i = 0; i < memory_params->num_frames; i++) {
        state->frame_table[i].page_num = -1;
        state->frame_table[i].in_use = false;
    }
    state->page_table = checked_malloc(memory_params->num_pages * sizeof(page_table_entry));
    for(int i = 0; i < memory_params->num_pages; i++) {
        state->page_table[i].frame_num = -1;
        state->page_table[i].present = false;
    }
    state->constants = memory_params;
    state->page_faults = 0;
    return state;
}

int common_state_free(common_pager_state* state) {
    if(state == NULL) {
        return -1;
    }
    free(state->frame_table);
    free(state->page_table);
    free(state);
    return 0;
}

int default_get_num_page_faults(pager_interface* self) {
    return self->common_state->page_faults;
}

bool default_check_table_contains(pager_interface* self, int virtual_address, int physical_address) {
    common_pager_state* state = self->common_state;
    int page_num = virtual_address / state->constants->page_size;
    int frame_num = physical_address / state->constants->page_size;
    bool frame_table_correct = state->frame_table[frame_num].in_use && state->frame_table[frame_num].page_num == page_num;
    bool page_table_correct = state->page_table[page_num].present && state->page_table[page_num].frame_num == frame_num;
    return frame_table_correct && page_table_correct;
}

int default_translate(pager_interface* self, int virtual_address) {
    common_pager_state* state = self->common_state;
    int page_num = virtual_address / state->constants->page_size;
    int offset = virtual_address % state->constants->page_size;
    int frame_num = -1;
    if(state->page_table[page_num].present) {
        frame_num = state->page_table[page_num].frame_num;
    } else {
        state->page_faults++;
        frame_num = self->handle_page_fault(self, page_num);
    }
    return frame_num * state->constants->page_size + offset;
}


void update_tables(common_pager_state* common_state, int faulting_page, int target_frame) {
    // If the target frame was already in use, update the entry for its former page to show it was evicted
    if(common_state->frame_table[target_frame].in_use) {
        common_state->page_table[common_state->frame_table[target_frame].page_num].present = false;
    }
    // Set the mapping in the page table
    common_state->page_table[faulting_page].frame_num = target_frame;
    common_state->page_table[faulting_page].present = true;
    // Update the frame table
    common_state->frame_table[target_frame].page_num = faulting_page;
    common_state->frame_table[target_frame].in_use = true;
}
