#include "pager.h"
#include "util/vector.h"
#include "util/checked_malloc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

typedef enum paging_algorithm {
    FIFO,
    LRU,
    RANDOM,
    CLOCK,
    OPT
} paging_algorithm;

pager_interface* build_pager(paging_algorithm type, const memory_constants* constants, const vector* address_trace) {
    pager_interface* pager;
    switch(type) {
        case FIFO:
            pager = fifo_pager_new(constants);
            break;
        case LRU:
            pager = lru_pager_new(constants);
            break;
        case CLOCK:
            pager = clock_pager_new(constants);
            break;
        case RANDOM:
            pager = random_pager_new(constants);
            break;
        case OPT:
            // The OPT pager gets a pointer to the address trace so it can see the future
            pager = opt_pager_new(constants, address_trace);
            break;
        default:
            printf("Invalid pager type! Cannot construct a pager\n");
            exit(EXIT_FAILURE);
    }
    return pager;
}

void free_pager(pager_interface* pager, paging_algorithm type) {
    switch(type) {
        case FIFO:
            fifo_pager_free(pager);
            break;
        case LRU:
            lru_pager_free(pager);
            break;
        case CLOCK:
            clock_pager_free(pager);
            break;
        case RANDOM:
            random_pager_free(pager);
            break;
        case OPT:
            opt_pager_free(pager);
            break;
    }
}

int main(int argc, char* argv[]) {
    if(argc < 5) {
        printf("Error: Missing required arguments\n");
        printf("Required arguments: [page size] [num frames] [algorithm] [trace file]\n");
        return EXIT_FAILURE;
    }
    // Parse the command-line arguments
    int page_size = atoi(argv[1]);
    if(page_size == 0) {
        printf("Error: Number of addresses per page (argument 1) must be an integer\n");
        return EXIT_FAILURE;
    }
    int num_frames = atoi(argv[2]);
    if(num_frames == 0) {
        printf("Error: Number of frames (argument 2) must be an integer\n");
        return EXIT_FAILURE;
    }
    char* algorithm_str = argv[3];
    paging_algorithm algorithm;
    if(strcmp(algorithm_str, "FIFO") == 0) {
        algorithm = FIFO;
    } else if(strcmp(algorithm_str, "LRU") == 0) {
        algorithm = LRU;
    } else if (strcmp(algorithm_str, "RANDOM") == 0) {
        algorithm = RANDOM;
    } else if (strcmp(algorithm_str, "CLOCK") == 0) {
        algorithm = CLOCK;
    } else if (strcmp(algorithm_str, "OPT") == 0) {
        algorithm = OPT;
    } else {
        printf("Error: Invalid algorithm. Choices are FIFO, LRU, RANDOM, CLOCK, OPT\n");
        return EXIT_FAILURE;
    }
    char* trace_file_name = argv[4];
    // Read and parse the trace file specified by argument 4
    FILE* trace_file = fopen(trace_file_name, "r");
    if(!trace_file) {
        printf("Error: Could not open trace file %s\n", trace_file_name);
        return EXIT_FAILURE;
    }
    vector* address_trace = vector_new();
    // Allocate this int on the heap so it can go in the vector
    int* address = checked_malloc(sizeof(*address));
    int max_address = 0;
    // Each line of the file is an integer followed by whitespace (a newline)
    while(fscanf(trace_file, "%d ", address) != EOF) {
        vector_push_back(address_trace, address);
        if(*address > max_address) {
            max_address = *address;
        }
        // Re-point the pointer to a new heap-allocated int for the next loop iteration
        address = checked_malloc(sizeof(*address));
    }
    fclose(trace_file);
    // After the last loop iteration, the last "address" is unused
    free(address);
    // Determine the number of pages of virtual memory based on the largest observed address
    int num_pages = max_address / page_size + 1;
    if(num_pages == 1) {
        printf("WARNING: Simulated virtual memory only has 1 page, so no paging will occur. "
            "Maximum virtual address in this trace is %d, so choose a page size smaller than %d "
            "to get multiple pages.\n", max_address, max_address);
    }
    memory_constants memory_params = {
        .num_frames = num_frames,
        .page_size = page_size,
        .num_pages = num_pages
    };
    printf("Starting simulation with %d pages, %d frames, page size = %d\n", num_pages, num_frames, page_size);
    // Construct a pager based on the requested algorithm
    pager_interface* pager = build_pager(algorithm, &memory_params, address_trace);
    // Run the simulation
    for(int i = 0; i < vector_size(address_trace); i++) {
        void* virtual_address_ptr;
        if(vector_get(address_trace, i, &virtual_address_ptr) != 0) {
            printf("Unexpected error: Failed to get entry %d from address trace!\n", i);
            return EXIT_FAILURE;
        }
        int virtual_address = *(int*)virtual_address_ptr;
        int physical_address = pager->translate(pager, virtual_address);
        assert(pager->check_table_contains(pager, virtual_address, physical_address));
    }
    printf("Total page faults with %s algorithm: %d\n", algorithm_str, pager->get_num_page_faults(pager));
    // Free heap memory (technically optional, since this is the end of the program)
    while(vector_size(address_trace) > 0) {
        void* address_ptr;
        if(vector_pop_back(address_trace, &address_ptr) == 0) {
            free(address_ptr);
        }
    }
    vector_free(address_trace);
    free_pager(pager, algorithm);
    return EXIT_SUCCESS;
}
