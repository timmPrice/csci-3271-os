#ifndef __VECTOR_H__
#define __VECTOR_H__

/**
 * A generic vector (variable-size array) that stores void* pointers.
 * The vector does not own the pointers it stores, so it is up to the caller to
 * properly allocate and free the objects they point to.
 */
typedef struct vector vector;

/**
 * Creates and returns a new, empty vector with default initial capacity.
 */
vector* vector_new();

/**
 * Resizes the vector to the specified size. If the new size is larger than the
 * current size, new array entries will be added at the end of the vector, with
 * values equal to NULL. If the new size is smaller than the current size, the
 * array entries at the end of the vector are removed, but this method will not
 * attempt to free() the pointers they contain. If the entries at the end of
 * the vector contain valid pointers, the caller must free the memory they
 * point to before calling this method to avoid a memory leak.
 *
 * @param v The vector to modify
 * @param new_size The size the vector should be after calling this function
 * @return 0 on success, -1 on failure
 */
int vector_resize(vector* v, int new_size);

/**
 * Adds a new item (pointer) to the end of the vector, increasing its size
 * by 1. This also increases its capacity if necessary.
 *
 * @param v The vector to modify
 * @param item A pointer to store at the end of the vector
 * @return 0 on success, -1 on failure
 */
int vector_push_back(vector* v, void* item);

/**
 * Removes and returns the last item (pointer) in the vector. The returned
 * pointer is placed in the output parameter *item; the return value of the
 * function indicates success or failure.
 *
 * @param v The vector to modify
 * @param out_item A pointer to a void* value in which the returned item-pointer
 * will be placed.
 * @return 0 on success, -1 on failure (e.g. the vector is empty)
 */
int vector_pop_back(vector* v, void** out_item);

/**
 * Returns a copy of the pointer at the specified position in the vector,
 * similar to the array access operator [] for an array of void*. The returned
 * pointer is placed in the output parameter *item; the return value of the
 * function indicates success or failure.
 *
 * @param v The vector to read from
 * @param index The index from which to read a pointer value
 * @param out_item A pointer to a void* value in which the returned item-pointer
 * will be placed
 * @return 0 on success, -1 on failure (e.g. index is not a valid index)
 */
int vector_get(const vector* v, int index, void** out_item);

/**
 * Sets the value at the specified position in the vector to new_item, similar
 * to the array access operator [] for an array of void*. This overwrites the
 * old value at that position without attempting to free() it; if the vector
 * previously contained a valid pointer at that position, the caller must free
 * the memory it points to before calling this function to avoid a memory leak.
 *
 * @param v The vector to modify
 * @param index The index at which to write a new pointer value
 * @param new_item A new pointer to store at the specified index
 * @return 0 on success, -1 on failure (e.g. index is not a valid index)
 */
int vector_set(vector* v, int index, void* new_item);

/**
 * Returns the size of the vector (current number of elements). This is not the
 * same as the capacity, which is an internal value managed by the vector.
 *
 * @param v The vector to examine
 * @return The size of the vector
 */
int vector_size(const vector* v);

/**
 * Frees the memory used by an empty vector. Fails if there are still items in
 * the vector.
 *
 * @param v The vector to free
 * @return 0 on success, -1 on failure
 */
int vector_free(vector* v);

#endif
