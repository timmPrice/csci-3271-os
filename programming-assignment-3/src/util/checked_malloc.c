#include "checked_malloc.h"

#include <stdlib.h>
#include <stdio.h>

void* checked_malloc(size_t size) {
    void* mem = malloc(size);
    if(mem == NULL) {
        fprintf(stderr, "Fatal error: Could not allocate memory of size %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    return mem;
}