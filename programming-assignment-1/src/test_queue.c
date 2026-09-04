#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

// A queue function to use in queue_iterate
void sum_ints_in_queue(void* queue_item, void* accumulator_arg) {
    int* item_as_int = queue_item;
    int* arg_as_int = accumulator_arg;
    *arg_as_int += *item_as_int;
}

// A queue function to test delete on empty queues
void empty_queue(queue* queue) {
    int an_item = 5;
    void* a_void = &an_item;

    int length = queue_length(queue);
    int delete_status = queue_delete(queue, &an_item);
    int dequeue_status = queue_dequeue(queue, &a_void);

    if (length != 0) {
        printf("queue returned the wrong length: %d\n", length);
    }

    if (delete_status != -1) {
        printf("queue deleted wrong item\n");
    }

    if (dequeue_status != -1) {
        printf("queue dequeued wrong item\n");
    }
}

int main() {
    queue* my_queue = queue_new();

    empty_queue(my_queue);

    int a = 1;
    int b = 2;
    int c = 3;
    // Enqueue pointers to local int variables
    int status = queue_enqueue(my_queue, &a);
    if (status != 0) {
        printf("Error! Failed to enqueue %d\n", a);
    }
    status = queue_enqueue(my_queue, &b);
    if (status != 0) {
        printf("Error! Failed to enqueue %d\n", b);
    }
    status = queue_enqueue(my_queue, &c);
    if (status != 0) {
        printf("Error! Failed to enqueue %d\n", c);
    }
    printf("Length of queue is %d\n", queue_length(my_queue));
    // Use queue_iterate to add up the numbers in the queue
    int sum = 0;
    queue_iterate(my_queue, &sum_ints_in_queue, &sum);
    printf("Sum of numbers in queue: %d\n", sum);
    // Dequeue and print each value
    while (queue_length(my_queue) > 0) {
        void* dequeued_item = NULL;
        queue_dequeue(my_queue, &dequeued_item);
        printf("Dequeued %d from the queue\n", *(int*)dequeued_item);
    }
    // Free the queue
    queue_free(my_queue);
}


