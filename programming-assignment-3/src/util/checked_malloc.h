#ifndef __CHECKED_MALLOC_H__
#define __CHECKED_MALLOC_H__

#include <stddef.h>

/**
 * A wrapper for the standard malloc() function that checks if the return value
 * is NULL. Prints an error message and exits with EXIT_FAILURE if malloc(size)
 * returns NULL, otherwise returns the return value of malloc(size). This is
 * useful in the common case where a malloc() failure is an unrecoverable error
 * and indicates a serious bug in the program.
 *
 * @param size The number of bytes of memory to allocate
 * @return A pointer to the allocated memory
 */
void* checked_malloc(size_t size);

#endif