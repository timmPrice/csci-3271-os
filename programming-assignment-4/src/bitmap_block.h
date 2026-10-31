/**
 * A set of functions for treating a disk block as a bitmap of BLOCK_SIZE * 8 bits.
 *
 */

#ifndef __BITMAP_BLOCK_H__
#define __BITMAP_BLOCK_H__

#include "disk.h"

/**
 * Zeroes out all bytes in a block, preparing it for use as a bitmap.
 *
 * @param block The block to modify.
 * @return 0 if the operation succeeded, -1 if it failed (because block was NULL)
 */
int bitmap_block_init(disk_block* block);

/**
 * Sets a bit to 1 within a disk block.
 *
 * @param block The block to modify.
 * @param index The bit index within the block to set. Valid indices are 0 to BLOCK_SIZE * 8 - 1.
 * @return 0 if the operation succeeded, -1 if it failed.
 */
int bitmap_block_set(disk_block* block, int index);

/**
 * Sets a bit to 0 within a disk block.
 *
 * @param block The block to modify.
 * @param index The bit index within the block to clear. Valid indices are 0 to BLOCK_SIZE * 8 - 1.
 * @return 0 if the operation succeeded, -1 if it failed.
 */
int bitmap_block_clear(disk_block* block, int index);

/**
 * Examines a bit within a disk block.
 *
 * @param block The block to read.
 * @param index The bit index within the block to test. Valid indices are 0 to BLOCK_SIZE * 8 - 1.
 * @return The value of the bit (0 or 1) if successful, or -1 if the operation failed.
 */
int bitmap_block_test(disk_block* block, int index);

/**
 * Finds the first bit within the block that is 0, sets it to 1, and returns
 * the index of the bit that was set. This is useful when the bitmap is used
 * to keep track of free and in-use objects, as it has the effect of allocating
 * a free object and returning the index number of the allocated object.
 *
 * @param block The block to read.
 * @return The index of a bit within the block that was set from 0 to 1, or -1
 * if the operation failed (because all bits in the block are already 1).
 */
int bitmap_block_allocate(disk_block* block);

#endif
