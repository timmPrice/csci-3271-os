#include "vector.h"

#include <stdlib.h>
#include <stdio.h>

#define SUCCESS 0
#define FAILURE -1

#define DEFAULT_CAPACITY 8

struct vector {
    int size;
    int capacity;
    /* An array of void* pointers, not a 2D array. */
    void** data;
};


vector* vector_new() {
    vector* new_vector = malloc(sizeof(*new_vector));
    if(!new_vector) {
	   printf("malloc failure: Could not get memory to create vector!\n");
        exit(EXIT_FAILURE);
    }
    new_vector->size = 0;
    new_vector->capacity = DEFAULT_CAPACITY;
    new_vector->data = malloc(DEFAULT_CAPACITY * sizeof(void*));
    if (!new_vector->data) {
	   printf("malloc failure: Could not get memory for initial vector storage!\n");
	   exit(EXIT_FAILURE);
    }
    return new_vector;
}

int vector_resize(vector* v, int new_size) {
    if(v == NULL || new_size < 0) {
        return FAILURE;
    }
    if(new_size > v->capacity) {
        // Increase capacity to match the new size
        void** new_array = realloc(v->data, new_size * sizeof(void*));
        if(new_array == NULL) {
            printf("Realloc error: Could not get memory to expand vector!\n");
            exit(EXIT_FAILURE);
        }
        v->data = new_array;
        v->capacity = new_size;
    }
    if(new_size > v->size) {
        // Zero-initialize new vector elements
        for(int i = v->size; i < new_size; i++) {
            v->data[i] = NULL;
        }
    }
    v->size = new_size;
    return SUCCESS;
}

int vector_push_back(vector* v, void* item) {
    if(v == NULL) {
        return FAILURE;
    }
    // Indices 0-(size-1) are occupied, index size is the first available
    // If size == capacity, the array is full since data[capacity] is an invalid index
    if(v->size >= v->capacity) {
        // Double the capacity, so repeated push_back doesn't cause repeated reallocations
        void** new_array = realloc(v->data, v->capacity * 2 * sizeof(void*));
        if(new_array == NULL) {
            printf("Realloc error: Could not get memory to expand vector!");
            exit(EXIT_FAILURE);
        }
        v->data = new_array;
        v->capacity *= 2;
    }
    v->data[v->size] = item;
    v->size++;
    return SUCCESS;
}

int vector_pop_back(vector* v, void** out_item) {
    if(v == NULL || out_item == NULL) {
        return FAILURE;
    }
    *out_item = v->data[v->size-1];
    v->size--;
    return SUCCESS;
}

int vector_get(const vector* v, int index, void** out_item) {
    if(v == NULL || out_item == NULL || index >= v->size || index < 0) {
        return FAILURE;
    }
    *out_item = v->data[index];
    return SUCCESS;
}

int vector_set(vector* v, int index, void* new_item) {
    if(v == NULL || index >= v->size || index < 0) {
        return FAILURE;
    }
    v->data[index] = new_item;
    return SUCCESS;
}

int vector_size(const vector* v) {
    if(v == NULL) {
        return FAILURE;
    }
    return v->size;
}

int vector_free(vector* v) {
    if(v == NULL || v->size > 0) {
        return FAILURE;
    }
    free(v->data);
    free(v);
    return SUCCESS;
}
