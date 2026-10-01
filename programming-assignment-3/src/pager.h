#ifndef __PAGER_H__
#define __PAGER_H__

#include "util/vector.h"

#include <stdlib.h>
#include <stdbool.h>

/**
 * The set of constants used to configure the parameters of the virtual memory
 * being simulated. These are set once at the beginning of the program and then
 * passed into the pager as constructor parameters; they should remain unchanged
 * throughout the life of a pager object.
 */
typedef struct memory_constants {
    const int page_size;
    const int num_frames;
    const int num_pages;
} memory_constants;

/**
 * A simple page table entry. Records the frame number and the "present" bit.
 * The index of this entry in the array is the page number.
 */
typedef struct page_table_entry {
    int frame_num;
    bool present;
} page_table_entry;

/**
 * An entry in the "frame table" used by the page table manager to keep track
 * of which frames in physical memory are available. If a frame is available,
 * its frame table entry will have in_use = false; if a frame is occupied by a
 * page, its frame table entry will have in_use = true and page_num equal to
 * the page stored in that frame.
 */
typedef struct frame_table_entry {
    int page_num;
    bool in_use;
} frame_table_entry;

typedef struct common_pager_state {
    /* An array of size constants->num_frames, with one entry per frame. */
    frame_table_entry* frame_table;
    /* An array of size constants->num_pages, with one entry per page. */
    page_table_entry* page_table;
    /* A pointer to the memory configuration constants. */
    const memory_constants* constants;
    /* Total number of page faults observed since the pager was constructed. */
    int page_faults;
} common_pager_state;

/**
 * The interface for a page table manager, or pager.
 */
typedef struct pager pager_interface;
struct pager {
    /**
     * A function that performs a virtual-to-physical address translation using
     * this page table manager. The function's parameter is the virtual address,
     * and its return value is the physical address.
     */
    int (*translate)(pager_interface* self, int virtual_address);
    /**
     * A function that handles a page fault using this page table manager. The
     * function's parameter is the page that caused the page fault, and its
     * return value is the frame number that this page has been assigned to
     * after handling the page fault.
     */
    int (*handle_page_fault)(pager_interface* self, int faulting_page);
    /**
     * A function that retrieves the total number of page faults observed by
     * this page table manager since the start of the simulation.
     */
    int (*get_num_page_faults)(pager_interface* self);
    /**
     * A function that checks whether the page table manager's tables contain a
     * mapping from a specified virtual address to a specified physical address.
     * Used in debugging to check the results of translate().
     */
    bool (*check_table_contains)(pager_interface* self, int virtual_address, int physical_address);
    /**
     * A struct containing common state variables needed by all implementations
     * of pager_interface. This page table manager owns the pointer.
     */
    common_pager_state* common_state;
    /**
     * An object of unknown type containing implementation-specific state needed
     * by this implementation of the pager interface. This page table manager
     * owns the pointer.
     */
    void* state;
};

/**
 * Creates and returns a new instance of common_pager_state
 *
 * @param memory_params The virtual memory configuration constants
 */
common_pager_state* common_state_new(const memory_constants* memory_params);

/**
 * Deletes an instance of common_pager_state.
 *
 * @return 0 on success, -1 on failure
 */
int common_state_free(common_pager_state* state);

/**
 * Default implementation of get_num_page_faults, using the counter in common_state.
 * If your pager implementation correctly updates the page_faults counter in
 * common_state, you can set get_num_page_faults to this function instead of
 * writing your own get_num_page_faults.
 *
 * @param self A pointer to the pager_interface instance that called this function
 * @return The number of page faults observed by the pager object
 */
int default_get_num_page_faults(pager_interface* self);

/**
 * Default implementation of check_table_contains, using the tables in common_state.
 * If your pager implementation correctly updates the page_table and frame_table in
 * common_state, you can set check_table_contains to this function instead of writing
 * your own function.
 *
 * @param self A pointer to the pager_interface instance that called this function
 * @param virtual_address The virtual address to check
 * @param physical_address The physical address to check
 * @return True if the virtual address maps to the physical address according to
 * page_table, and the physical frame contains the virtual address according to
 * frame_table.
 */
bool default_check_table_contains(pager_interface* self, int virtual_address, int physical_address);

/**
 * Default implementation of translate, using the tables in common_state. If
 * the requested page is not present according to page_table, this function calls
 * self->handle_page_fault, then completes the translation using the frame returned
 * by handle_page_fault. It assumes that handle_page_fault also updates the page
 * table and frame table.
 *
 * @param self A pointer to the pager_interface instance that called this function
 * @param virtual_address The requested virtual address
 * @return The physical address corresponding to virtual_address
 */
int default_translate(pager_interface* self, int virtual_address);

/**
 * Helper function that can be used as part of handle_page_fault to update
 * common_state's page table and frame table once a frame has been chosen
 * for the faulting page.
 *
 * @param common_state A pointer to the common_state held by a pager_interface
 * instance.
 * @param faulting_page The page number of the page that has been newly loaded
 * into memory at target_frame
 * @param target_frame The frame number of the frame that is now occupied by
 * faulting_page
 */
void update_tables(common_pager_state* common_state, int faulting_page, int target_frame);

/* Constructor and free methods for each implementation of pager_interface */

pager_interface* dummy_pager_new(const memory_constants* memory_params);
int dummy_pager_free(pager_interface* self);

pager_interface* fifo_pager_new(const memory_constants* memory_params);
int fifo_pager_free(pager_interface* self);

pager_interface* lru_pager_new(const memory_constants* memory_params);
int lru_pager_free(pager_interface* self);

pager_interface* clock_pager_new(const memory_constants* memory_params);
int clock_pager_free(pager_interface* self);

pager_interface* random_pager_new(const memory_constants* memory_params);
int random_pager_free(pager_interface* self);

/* 
 * Unlike all the other pagers, the OPT pager constructor gets an extra 
 * parameter: a pointer to the address trace vector used by the main simulation.
 * The OPT pager should not modify this vector, but it can read it to look 
 * ahead at which addresses will be used in the future.
 */
pager_interface* opt_pager_new(const memory_constants* memory_params, const vector* address_trace);
int opt_pager_free(pager_interface* self);

#endif
