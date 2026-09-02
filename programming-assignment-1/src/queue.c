/*
 * Generic queue implementation.
 */ 

#include "queue.h"
#include <stdlib.h>

typedef struct item {
    void* item;
    struct item* next;
} item;

struct queue {
    struct item* head;
    struct item* tail;
    int size;
};

queue* queue_new() {
    queue* q = malloc(sizeof(queue));

    q -> head = NULL;
    q -> tail = NULL;
    q -> size = 0;

    if (q != NULL) {
        return q;
    }

    return NULL;
}

int queue_enqueue(queue* queue, void* data) {
   
    struct item* newItem = malloc(sizeof(item));

    if (newItem == NULL) {
        return -1;
    }

    newItem -> next = NULL;

    newItem -> item = data;

    if (queue -> tail == NULL) {
        queue -> head = newItem;
        queue -> tail = newItem;
    } else {
        queue -> tail -> next = newItem;
        queue -> tail = newItem;
    }

    queue -> size = queue -> size++;
    return 0;
}

int queue_dequeue(queue* queue, void** item) {
    return -1;
}

int queue_iterate(queue* queue, queue_func f, void* arg) {
    return -1;
}

int queue_free(queue* queue) {
    return -1;
}

int queue_length(const queue* queue) {
    return -1;
}

int queue_delete(queue* queue, void* item) {
    return -1;
}
