#include "random.h"

#include <stdlib.h>
#include <assert.h>

int random_limit(int limit) {
    assert(limit > 0);
    assert(limit <= RAND_MAX);
    int even_divide_limit = RAND_MAX - (RAND_MAX % limit);
    int sample;
    do {
        sample = rand();
    } while (sample >= even_divide_limit);
    return sample % limit;
}

int random_range(int low, int high) {
    return random_limit(high - low) + low;
}

void random_string(char* destination, size_t size) {
    for (int i = 0; i < size - 1; ++i) {
        // Pick a random ASCII value between the first and last printable characters
        destination[i] = (char)random_range('!', '~');
    }
    // Null-terminate the string
    destination[size - 1] = 0;
}