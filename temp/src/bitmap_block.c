#include "bitmap_block.h"

#include <stdint.h>
#include <stdlib.h>

int bitmap_block_init(disk_block* block) {
    if (block == NULL) {
        return -1;
    }
    for (int i = 0; i < BLOCK_SIZE; i++) {
        block->bytes[i] = 0;
    }
    return 0;
}

int bitmap_block_set(disk_block* block, int index) {
    if (block == NULL || index < 0 || index >= BLOCK_SIZE * 8) {
        return -1;
    }
    uint8_t mask = 1U << (index % 8);
    block->bytes[index / 8] |= mask;
    return 0;
}

int bitmap_block_clear(disk_block* block, int index) {
    if (block == NULL || index < 0 || index >= BLOCK_SIZE * 8) {
        return -1;
    }
    uint8_t mask = ~(1U << (index % 8));
    block->bytes[index / 8] &= mask;
    return 0;
}

int bitmap_block_test(disk_block* block, int index) {
    if (block == NULL || index < 0 || index >= BLOCK_SIZE * 8) {
        return -1;
    }
    uint8_t mask = 1U << (index % 8);
    if (block->bytes[index / 8] & mask) {
        return 1;
    } else {
        return 0;
    }
}

int bitmap_block_allocate(disk_block* block) {
    if (block == NULL) {
        return -1;
    }
    for (int i = 0; i < BLOCK_SIZE * 8; i++) {
        if (bitmap_block_test(block, i) == 0) {
            bitmap_block_set(block, i);
            return i;
        }
    }
    return -1;
}
