#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <stddef.h>

/**
 * Returns a random number in the range [0, limit).
 *
 * @param limit The upper limit of the random number range, exclusive.
 * @return The random number.
 */
int random_limit(int limit);

/**
 * Returns a random number in the range [low, high)
 *
 * @param low The lower limit of the random number range, inclusive.
 * @param high The upper limit of the random number range, exclusive.
 * @return The random number.
 */
int random_range(int low, int high);

/**
 * Fills the character array pointed to by destination with a random printable,
 * null-terminated string.
 *
 * @param destination The character array to populate.
 * @param size The size of the character array.
 */
void random_string(char* destination, size_t size);

#endif
