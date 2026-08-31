#include "vector.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
    vector* my_vector = vector_new();
    int a = 1;
    int b = 2;
    int c = 3;
    // Insert pointers to local int variables
    int status = vector_push_back(my_vector, &a);
    if (status != 0) {
        printf("Error! Failed to push_back %d\n", a);
    }
    printf("Size of vector is %d\n", vector_size(my_vector));
    status = vector_push_back(my_vector, &b);
    if (status != 0) {
        printf("Error! Failed to push_back %d\n", b);
    }
    printf("Size of vector is %d\n", vector_size(my_vector));
    status = vector_push_back(my_vector, &c);
    if (status != 0) {
        printf("Error! Failed to push_back %d\n", c);
    }
    printf("Size of vector is %d\n", vector_size(my_vector));
    // Add a pointer to a heap-allocated int
    int* d = malloc(sizeof(int));
    *d = 4;
    status = vector_push_back(my_vector, d);
    if (status != 0) {
        printf("Error! Failed to push_back %d\n", *d);
    }
    printf("Size of vector is %d\n", vector_size(my_vector));
    // Loop over the elements of the vector and add them up
    int sum = 0;
    for (int i = 0; i < vector_size(my_vector); ++i) {
	   void* item = NULL;
	   status = vector_get(my_vector, i, &item);
	   if (status != 0) {
		  printf("Error! Failed to get pointer at index %d\n", i);
	   }
        // Cast to int* and dereference
	   sum += *(int*)item;
    }
    printf("Sum of numbers in vector: %d\n", sum);
    // Remove and print each value
    while (vector_size(my_vector) > 0) {
        void* removed_item = NULL;
        vector_pop_back(my_vector, &removed_item);
        printf("Removed %d from the vector\n", *(int*)removed_item);
    }
    // Delete the heap-allocated int
    free(d);
    if (vector_free(my_vector) != 0) {
        printf("Error! Failed to free the vector\n");
    }
}