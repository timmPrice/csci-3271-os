#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>

/**
 * A generic linked list of void* pointers. The list does not own the pointers
 * it stores (and does not know their types) so it is up to the caller to
 * properly allocate and free the objects they point to.
 */
typedef struct list list;

/**
 * A function-pointer type for an equality comparison function between two list items.
 */
typedef bool (*equals_func)(void* item1, void* item2);

/**
 * Creates and returns a new, empty linked list.
 */
list* list_new();

/**
 * Adds a new pointer to the beginning of the linked list.
 *
 * @param list The list to modify
 * @param item A pointer to the item to add to the list
 * @return 0 on success, -1 on failure
 */
int list_add_first(list* list, void* item);

/**
 * Adds a new pointer to the end of the linked list.
 *
 * @param list The list to modify
 * @param item A pointer to the item to add to the list
 * @return 0 on success, -1 on failure
 */
int list_add_last(list* list, void* item);

/**
 * Removes and returns the first item (pointer) from the list. The returned
 * pointer is placed in the output parameter *item; the return value of the
 * function indicates success or failure.
 *
 * @param list The list to modify
 * @param item A pointer to a void* value in which the returned item-pointer
 * will be placed.
 * @return 0 on success, -1 on failure (e.g. the list is empty)
 */
int list_remove_first(list* list, void** item);

/**
 * Removes and returns the last item (pointer) from the list. The returned
 * pointer is placed in the output parameter *item; the return value of the
 * function indicates success or failure.
 *
 * @param list The list to modify
 * @param item A pointer to a void* value in which the returned item-pointer
 * will be placed.
 * @return 0 on success, -1 on failure (e.g. the list is empty)
 */
int list_remove_last(list* list, void** item);

/**
 * Deletes the element from the list whose value matches item according
 * to the equality-comparison function is_equal. This calls free() on the
 * pointer in the list, since it is not returned to the caller and would
 * otherwise be lost. Due to this limitation, this function should not be used
 * if the items in the list are complex objects with their own "free" functions
 * (like list itself).
 *
 * @param list The list to modify
 * @param item A pointer to an item to search for and remove from the list.
 * Must not be NULL.
 * @param is_equal A function used to compare each element in the list with the
 * parameter item. The first list element x for which is_equal(x, item) returns
 * true will be deleted.
 * @return 0 on success, -1 on failure (e.g. the requested item wasn't found)
 */
int list_delete(list* list, void* item, equals_func is_equal);

/**
 * Returns the length of the list, or -1 if the list argument is NULL.
 *
 * @param list The list to examine
 * @return The length of the list, in elements, or -1
 */
int list_length(const list* list);

/**
 * Frees the memory used by an empty linked list. Fails if the list is not
 * empty. (Each item in the list must be deleted by the caller first).
 *
 * @param list The list to free
 * @return 0 on success, -1 on failure
 */
int list_free(list* list);

#endif
