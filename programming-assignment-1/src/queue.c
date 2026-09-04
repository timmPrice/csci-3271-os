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

    queue -> size++;
    return 0;
}

int queue_dequeue(queue* queue, void** data) {
    if (queue -> head == NULL || queue -> size == 0) {
        return -1;
    } 

    *data = queue -> head -> item;
    item* aHead = queue -> head;

    queue -> head = queue -> head -> next; 

    if (queue -> head == NULL) {
        queue -> tail = NULL;  
    }

    free(aHead);
    queue -> size--;

    return 0;
}

int queue_iterate(queue* queue, queue_func f, void* arg) {
    if (queue == NULL || f == NULL){
        return -1;
    }
   
    item* current = queue -> head;
    while (current != NULL ) {
        f(current -> item, arg);
        current = current -> next;
    }

    return 0;
}

int queue_free(queue* queue) {
    if (queue -> tail != NULL || queue -> size != 0) {
        return -1;
    }
    free (queue);
    return 0;
}

int queue_length(const queue* queue) {
    if (queue == NULL) {
        return -1;
    }

    return queue -> size;
}

int queue_delete(queue* queue, void* the_item) {
    if (queue == NULL) {
        return -1;
    }
   
    item* current = queue -> head;
    item* prev;

    while (current != NULL) {
        if (current -> item == the_item) {
           if (current == queue -> head) {
                queue -> head = queue -> head -> next; 
                free(current);

                if (queue -> head == NULL) {
                    queue -> tail = NULL;
                }
    
                queue -> size--;
                return 0;
           } else {
                prev -> next = current -> next;

                if (current == queue -> tail) {
                    queue -> tail = prev;     
                }

                free(current);
                queue -> size--;
                return 0;
           }
        }

        prev = current;
        current = current -> next;
    }
    return -1;

}
