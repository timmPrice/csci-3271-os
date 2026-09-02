/*
 * Generic queue interface
 */
#ifndef __QUEUE_H__
#define __QUEUE_H__

/*
 * queue is the type of an internally maintained data structure.
 * Clients of this package do not need to know how queues are
 * represented.  They see and manipulate only pointers to queues.
 */
typedef struct queue queue;

/*
 * Return an empty queue.  Returns NULL on error.
 */
queue* queue_new();

/*
 * Append a void* item to the queue.
 * Returns 0 (success) or -1 (failure).
 */
int queue_enqueue(queue* queue, void* item);

/*
 * Dequeue and return the first void* item from the queue.
 * Returns 0 (success) and sets *item to the first item if the queue is 
 * nonempty; returns -1 (failure) and sets *item to NULL if the queue is empty.
 */
int queue_dequeue(queue* queue, void** item);

/*
 * Given a function pointer and an argument for the function, calls the 
 * function with that argument on each element in the queue. Specifically,
 * queue_iterate(q, f, t) calls f(x,t) for each x in q. q and f should be 
 * non-null, but t (the argument) may be null if it is not used by f.
 * Returns 0 (success) or -1 (failure).
 */
typedef void (*queue_func)(void*, void*);
int queue_iterate(queue* queue, queue_func f, void* arg);

/*
 * Free the queue and return 0 (success) or -1 (failure).
 * Can only be called on an empty queue; returns -1 if the queue is non-empty.
 */
int queue_free(queue* queue);

/*
 * Return the number of items in the queue, or -1 if an error occurred
 */
int queue_length(const queue* queue);

/*
 * Delete the first instance of the specified item from the given queue. Note
 * that this will compare pointers for equality, not the values they point to,
 * since the queue stores void* pointers (which cannot be dereferenced without 
 * casting).
 * Returns 0 (success) if an element was deleted, or -1 otherwise.
 */
int queue_delete(queue* queue, void* item);

#endif /*__QUEUE_H__*/