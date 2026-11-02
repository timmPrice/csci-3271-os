#ifndef __DISK_H__
#define __DISK_H__

#include <stdint.h>

#define BLOCK_SIZE 512
/**
 * Type alias for block numbers, which are unsigned integers
 */
typedef uint32_t blocknum;

/**
 * Data type representing a block, which is just a fixed-size array of bytes.
 * This wraps the array in a struct for type safety and naming.
 */
typedef struct disk_block {
    uint8_t bytes[BLOCK_SIZE];
} disk_block;

/**
 * The interface for a disk object with a block device interface. Any disk
 * implementation will have these functions.
 */
typedef struct disk_interface disk_interface;
struct disk_interface {
    /**
     * Reads a block from a disk into the block-size memory area pointed to by
     * `block`.
     *
     * @param self The disk to read from
     * @param block_num The block number on this disk to read.
     * @param block A pointer to a disk_block that will be overwritten by the
     * contents of the desired block.
     * @return 0 if the operation succeeded, -1 if it failed.
     */
    int (*read_block)(disk_interface* self, blocknum block_num, disk_block* block);
    /**
     * Writes a block from memory to the specified location on a disk.
     *
     * @param self The disk to write to.
     * @param block_num The block number on the disk to write to.
     * @param block A pointer to a disk_block in memory whose contents will be used
     * to overwrite the block on disk.
     * @return 0 if the operation succeeded, -1 if it failed.
     */
    int (*write_block)(disk_interface* self, blocknum block_num, const disk_block* block);
    /**
     * Returns the total number of blocks in a disk.
     *
     * @param self The disk to inspect.
     * @return The total number of blocks in the disk, or -1 if the argument is NULL.
     */
    int (*get_size)(disk_interface* self);
    /**
     * Shuts down a disk, releasing all memory resources associated with it. If the
     * disk is in-memory only, this deletes everything on the disk.
     *
     * @param self The disk to shut down
     * @return 0 if the operation succeeded, -1 if the argument is NULL.
     */
    int (*shutdown)(disk_interface* self);
    /**
     * Pointer to internal state associated with a disk interface implementation.
     */
    void* state;
};

/**
 * Creates a new in-memory disk object, allocating a region of memory to store
 * the blocks in.
 *
 * @param desired_size The number of blocks the new disk should contains
 * @return A pointer to the disk interface object for the memory disk.
 */
disk_interface* memory_disk_new(blocknum desired_size);

/**
 * Creates a new file-backed disk object that stores its blocks in the specified
 * file on the underlying file system.
 *
 * @param desired_size The number of blocks the disk should contain.
 * @param filename The name of the file to store the disk in.
 * @return A pointer to the disk interface object for the file disk.
 */
disk_interface* file_disk_new(blocknum desired_size, const char* filename);


#endif
