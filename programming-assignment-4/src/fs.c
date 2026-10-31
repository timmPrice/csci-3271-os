#include "fs.h"
#include "disk.h"
#include "bitmap_block.h"
#include "util/vector.h"
#include "util/checked_malloc.h"
#include "util/signed_size.h"
#include "util/string_util.h"
#include "util/min_max.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// The root directory has a fixed location: inode 1 and data block 0
#define ROOT_INODE 1
// Inode 0 is not used, and an inode number of 0 indicates a nonexistant inode
#define INVALID_INODE 0
// The root directory uses data block 0, but for everyone else it's invalid
#define INVALID_DATA_BLOCK 0

/* In-memory filesystem state, "private" to this module */

/**
 * The table of open files, stored as a vector of pointers to file_table_entry.
 * The index of a pointer is the file descriptor, and if the pointer is NULL the
 * file descriptor with that number has been closed (and can be reused).
 */
vector* file_table = NULL;
/**
 * The disk object on which this filesystem is being stored. All filesystem
 * operations after vsfs_init or vsfs_load use this disk to read and write blocks.
 */
disk_interface* the_disk = NULL;
/**
 * A snapshot of the superblock state stored on the superblock of the disk.
 * This does not change during filesystem execution, so it can be cached in
 * memory after being intialized or read.
 */
metadata_block superblock_snapshot;

/* Forward declarations of "private" helper functions not part of the interface in fs.h */

inum vsfs_traverse_to_inode(const char* path);
int vsfs_new_file_descriptor(inum inode_num, file_mode mode);
int vsfs_create_file(const char* path, file_type type, file_mode mode);

int vsfs_init(disk_interface* disk, int num_inodes) {
    int num_blocks = disk->get_size(disk);
    if (num_blocks < 0) {
        fprintf(stderr, "ERROR [%s]: Disk not initialized.\n", __func__);
        return -1;
    }
    if (num_inodes > BLOCK_SIZE * 8) {
        printf("WARNING: Requested number of inodes is too large. Only %d inodes can be covered by the free inode bitmap, but %d will be created.\n", BLOCK_SIZE * 8, num_inodes);
    }
    int inode_table_blocks = num_inodes / INODES_PER_BLOCK;
    int min_disk_blocks = 3 + inode_table_blocks + 1;
    if (num_blocks < min_disk_blocks) {
        fprintf(stderr, "ERROR [%s]: Disk size is too small. Only %d blocks found, need at least %d.\n", __func__, num_blocks, min_disk_blocks);
        return -1;
    }
    the_disk = disk;
    // Create the superblock
    metadata_block super_block;
    super_block.super.magic_number = VSFS_MAGIC_NUMBER;
    super_block.super.inode_bitmap_block_num = 1U;
    super_block.super.data_bitmap_block_num = 2U;
    super_block.super.first_inode_block_num = 3U;
    super_block.super.first_data_block_num = 3U + inode_table_blocks;
    super_block.super.num_data_blocks = num_blocks - super_block.super.first_data_block_num;
    if (super_block.super.num_data_blocks > BLOCK_SIZE * 8) {
        printf("WARNING: Disk size is too large. Only %d data blocks are covered by the free block bitmap, but %d are available.\n", BLOCK_SIZE * 8, super_block.super.num_data_blocks);
    }
    int status = the_disk->write_block(the_disk, 0, (disk_block*)&super_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to write superblock to the disk.\n", __func__);
        return -1;
    }
    // Save a copy of the superblock in memory
    superblock_snapshot = super_block;

    // Create a bitmap block for the inode bitmap
    disk_block inode_bitmap_block;
    bitmap_block_init(&inode_bitmap_block);
    status = bitmap_block_set(&inode_bitmap_block, ROOT_INODE);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to set bit %d in a bitmap block.\n", __func__, ROOT_INODE);
        return -1;
    }
    // The reserved "invalid" inode number should already be "in use" so it is not allocated
    status = bitmap_block_set(&inode_bitmap_block, INVALID_INODE);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to set bit %d in a bitmap block.\n", __func__, INVALID_INODE);
        return -1;
    }
    status = the_disk->write_block(the_disk, super_block.super.inode_bitmap_block_num, &inode_bitmap_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to write inode bitmap block to the disk.\n", __func__);
        return -1;
    }

    // Create a bitmap block for the data bitmap, with the first block allocated for the root directory
    disk_block data_bitmap_block;
    bitmap_block_init(&data_bitmap_block);
    status = bitmap_block_set(&data_bitmap_block, 0);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to set bit %d in a bitmap block.\n", __func__, 0);
        return -1;
    }
    status = the_disk->write_block(the_disk, super_block.super.data_bitmap_block_num, &data_bitmap_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to write data bitmap block to the disk.\n", __func__);
        return -1;
    }

    // Create the inode table
    for (int b = 0; b < inode_table_blocks; ++b) {
        metadata_block inode_table_block;
        for (size_t i = 0; i < INODES_PER_BLOCK; ++i) {
            inode_table_block.inode_table.inodes[i].type = FILE_TYPE_FREE;
            inode_table_block.inode_table.inodes[i].size = 0;
            inode_table_block.inode_table.inodes[i].link_count = 0;
            for (int j = 0; j < NUM_DIRECT_BLOCKS; ++j) {
                inode_table_block.inode_table.inodes[i].direct_blocks[j] = 0;
            }
            inode_table_block.inode_table.inodes[i].indirect_block = 0;
        }
        // If this is the first block, initialize the root directory's inode
        if (b == 0) {
            inode_table_block.inode_table.inodes[ROOT_INODE].type = FILE_TYPE_DIRECTORY;
            inode_table_block.inode_table.inodes[ROOT_INODE].size = BLOCK_SIZE;
            inode_table_block.inode_table.inodes[ROOT_INODE].link_count = 1;
            inode_table_block.inode_table.inodes[ROOT_INODE].direct_blocks[0] = 0;
        }
        status = the_disk->write_block(the_disk, super_block.super.first_inode_block_num + b, (disk_block*)&inode_table_block);
        if (status != 0) {
            fprintf(stderr, "ERROR [%s]: Failed to write inode table block %d to the disk.\n", __func__, super_block.super.first_inode_block_num + b);
            return -1;
        }
    }

    // Create the directory data block for the root directory, at the first data block
    metadata_block root_directory_block;
    root_directory_block.directory.entries[0].inode_num = ROOT_INODE;
    strcpy(root_directory_block.directory.entries[0].name, ".");
    for (size_t i = 1; i < DIRECTORY_ENTRIES_PER_BLOCK; ++i) {
        root_directory_block.directory.entries[i].inode_num = INVALID_INODE;
        memset(root_directory_block.directory.entries[i].name, '\0', MAX_FILE_NAME_LENGTH);
    }
    status = the_disk->write_block(the_disk, super_block.super.first_data_block_num, (disk_block*)&root_directory_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to write directory data block to the disk.\n", __func__);
        return -1;
    }

    // Create the in-memory file table (initally empty)
    file_table = vector_new();

    return 0;
}

int vsfs_load(disk_interface* disk) {
    int num_disk_blocks = disk->get_size(disk);
    if (num_disk_blocks < 0) {
        fprintf(stderr, "ERROR [%s]: Disk not initialized.\n", __func__);
        return -1;
    }
    the_disk = disk;
    int status = the_disk->read_block(the_disk, 0, (disk_block*) &superblock_snapshot);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to read superblock from the disk.\n", __func__);
        return -1;
    }
    // Basic sanity checks
    if (superblock_snapshot.super.magic_number != VSFS_MAGIC_NUMBER) {
        fprintf(stderr, "ERROR [%s]: Superblock did not have a valid magic number. Not a VSFS file system.\n", __func__);
        return -1;
    }
    int total_blocks = superblock_snapshot.super.first_data_block_num + superblock_snapshot.super.num_data_blocks;
    if (total_blocks > num_disk_blocks) {
        fprintf(stderr, "ERROR [%s]: Filesystem described by superblock is larger than the disk. Filesystem should have %d blocks but disk is only %d blocks.\n", __func__, total_blocks, num_disk_blocks);
        return -1;
    }

    file_table = vector_new();
    return 0;
}

int vsfs_link(const char* source_path, const char* dest_path) {

    return -1;
}

int vsfs_unlink(const char* path) {

    return -1;
}

int vsfs_create(const char* path, file_mode mode) {
    if (the_disk == NULL) {
        fprintf(stderr, "ERROR [%s]: Filesystem has not been initialized.\n", __func__);
        return -1;
    }
    if (path == NULL) {
        fprintf(stderr, "ERROR [%s]: Invalid argument. Path cannot be NULL.\n", __func__);
        return -1;
    }
    return vsfs_create_file(path, FILE_TYPE_REGULAR, mode);
}

int vsfs_mkdir(const char* path) {
    if (the_disk == NULL) {
        fprintf(stderr, "ERROR [%s]: Filesystem has not been initialized.\n", __func__);
        return -1;
    }
    if (path == NULL) {
        fprintf(stderr, "ERROR [%s]: Invalid argument. Path cannot be NULL.\n", __func__);
        return -1;
    }
    return vsfs_create_file(path, FILE_TYPE_DIRECTORY, FILE_MODE_READ);
}

/**
 * Internal function that creates either a file or a directory depending on
 * the arguments. Most of the code for creating these is the same, and differs
 * only in how the new inode is initialized.
 * @param path The absolute path to the file or directory; its parent must exist
 * @param type A file_type indicating whether to create a file or directory
 * @param mode The mode to open the new file in, if type is FILE_TYPE_REGULAR
 * @return The file descriptor of the open file, or 0 to indicate success at
 * creating a directory, or -1 if there was an error.
 */
int vsfs_create_file(const char* path, file_type type, file_mode mode) {
    // Ensure there is a free inode for the new file
    disk_block inode_bitmap_block;
    int status = the_disk->read_block(the_disk, superblock_snapshot.super.inode_bitmap_block_num, &inode_bitmap_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to read the inode bitmap block from the disk.\n", __func__);
        return -1;
    }
    int new_inode_num = bitmap_block_allocate(&inode_bitmap_block);
    if (new_inode_num < 0) {
        fprintf(stderr, "ERROR [%s]: Filesystem is out of free inodes. Cannot create file.\n", __func__);
        return -1;
    }

    // Find the parent directory's inode
    char* parent_path = checked_malloc(strlen(path));
    status = path_parent(path, parent_path);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Could not parse a parent directory in path '%s'\n", __func__, path);
        free(parent_path);
        return -1;
    }
    inum parent_inode_num = vsfs_traverse_to_inode(parent_path);
    if (parent_inode_num == INVALID_INODE) {
        fprintf(stderr, "ERROR [%s]: Could not find an inode for path '%s'\n", __func__, parent_path);
        free(parent_path);
        return -1;
    }
    free(parent_path);
    blocknum parent_inode_block_num = superblock_snapshot.super.first_inode_block_num
        + (parent_inode_num / INODES_PER_BLOCK);
    inum parent_inode_offset = parent_inode_num % INODES_PER_BLOCK;
    metadata_block inode_table_block;
    status = the_disk->read_block(the_disk, parent_inode_block_num, (disk_block*)&inode_table_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to read inode table block %d from the disk.\n", __func__, parent_inode_block_num);
        return -1;
    }
    inode* parent_inode = &inode_table_block.inode_table.inodes[parent_inode_offset];
    if (parent_inode->type != FILE_TYPE_DIRECTORY) {
        fprintf(stderr, "ERROR [%s]: Parent of %s is not a directory\n", __func__, path);
        return -1;
    }
    // Read the data block and add a new entry to the parent directory
    metadata_block directory_data_block;
    status = the_disk->read_block(the_disk, superblock_snapshot.super.first_data_block_num
        + parent_inode->direct_blocks[0], (disk_block*)&directory_data_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to read block %d from the disk.\n", __func__, superblock_snapshot.super.first_data_block_num + parent_inode->direct_blocks[0]);
        return -1;
    }
    size_t entry_num = 0;
    while (directory_data_block.directory.entries[entry_num].inode_num != INVALID_INODE) {
        entry_num++;
        // Directories can only have 1 data block, so it's an error when the block is full
        if (entry_num == DIRECTORY_ENTRIES_PER_BLOCK) {
            fprintf(stderr, "ERROR [%s]: Create failed, parent directory is full\n", __func__);
            return -1;
        }
    }
    directory_data_block.directory.entries[entry_num].inode_num = new_inode_num;
    char* filename = checked_malloc(strlen(path));
    path_basename(path, filename);
    if (strlen(filename) > MAX_FILE_NAME_LENGTH) {
        fprintf(stderr, "ERROR [%s]: File name too long in path '%s'\n", __func__, path);
        free(filename);
        return -1;
    }
    strcpy(directory_data_block.directory.entries[entry_num].name, filename);
    free(filename);
    status = the_disk->write_block(the_disk, superblock_snapshot.super.first_data_block_num
        + parent_inode->direct_blocks[0], (disk_block*)&directory_data_block);

    // Write the inode bitmap block, marking the new file's inode as in-use
    status = the_disk->write_block(the_disk, superblock_snapshot.super.inode_bitmap_block_num, &inode_bitmap_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to write the inode bitmap block to the disk.\n", __func__);
        return -1;
    }
    // Initialize the inode for the new file. Note that it may be in the same block as the parent directory's inode.
    blocknum new_inode_block_num = superblock_snapshot.super.first_inode_block_num
        + (new_inode_num / INODES_PER_BLOCK);
    int new_inode_offset = new_inode_num % INODES_PER_BLOCK;
    metadata_block new_inode_block;
    status = the_disk->read_block(the_disk, new_inode_block_num, (disk_block*)&new_inode_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to read inode table block %d from the disk.\n", __func__, new_inode_block_num);
        return -1;
    }
    inode* new_inode = &new_inode_block.inode_table.inodes[new_inode_offset];
    new_inode->type = type;
    new_inode->size = 0;
    new_inode->link_count = 1;
    for (size_t i = 0; i < NUM_DIRECT_BLOCKS; ++i) {
        new_inode->direct_blocks[i] = INVALID_DATA_BLOCK;
    }
    new_inode->indirect_block = INVALID_DATA_BLOCK;
    // The inode may need further initialization if the new file is a directory, so don't write it yet

    // If the new file is a regular file, open it and return the file descriptor
    if (type == FILE_TYPE_REGULAR) {
        status = the_disk->write_block(the_disk, new_inode_block_num, (disk_block*)&new_inode_block);
        if (status != 0) {
            fprintf(stderr, "ERROR [%s]: Failed to write inode table block %d to the disk.\n", __func__, new_inode_block_num);
            return -1;
        }
        int fd = vsfs_new_file_descriptor(new_inode_num, mode);
        if (fd < 0) {
            fprintf(stderr, "ERROR [%s]: Failed to allocate a new file descriptor.\n", __func__);
            return -1;
        }
        return fd;
    }
    // If the new file is a directory, initialize the directory with the two default entries
    // This requires allocating a data block for the directory and linking it to the inode.
    disk_block data_bitmap;
    status = the_disk->read_block(the_disk, superblock_snapshot.super.data_bitmap_block_num, &data_bitmap);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to read data bitmap block from the disk.\n", __func__);
        return -1;
    }
    int dir_data_block_num = bitmap_block_allocate(&data_bitmap);
    if (dir_data_block_num < 0) {
        fprintf(stderr, "ERROR [%s]: Disk is full. No more data blocks available for new directory.\n", __func__);
        return -1;
    }
    status = the_disk->write_block(the_disk, superblock_snapshot.super.data_bitmap_block_num, &data_bitmap);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to write data bitmap block to the disk.\n", __func__);
        return -1;
    }
    metadata_block new_directory_block;
    new_directory_block.directory.entries[0].inode_num = new_inode_num;
    strcpy(new_directory_block.directory.entries[0].name, ".");
    new_directory_block.directory.entries[1].inode_num = parent_inode_num;
    strcpy(new_directory_block.directory.entries[1].name, "..");
    for (size_t i = 2; i < DIRECTORY_ENTRIES_PER_BLOCK; ++i) {
        new_directory_block.directory.entries[i].inode_num = INVALID_INODE;
        memset(new_directory_block.directory.entries[i].name, '\0', MAX_FILE_NAME_LENGTH);
    }
    status = the_disk->write_block(the_disk, superblock_snapshot.super.first_data_block_num + dir_data_block_num, (disk_block*)&new_directory_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to write directory data block %u to the disk.\n", __func__, superblock_snapshot.super.first_data_block_num + dir_data_block_num);
        return -1;
    }
    // Update the directory's inode and write it
    new_inode->direct_blocks[0] = dir_data_block_num;
    // Directories always have a size equal to the entire block since the table can be sparse
    new_inode->size = BLOCK_SIZE;
    status = the_disk->write_block(the_disk, new_inode_block_num, (disk_block*)&new_inode_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to write inode table block %u to the disk.\n", __func__, new_inode_block_num);
        return -1;
    }
    // Return 0 for success at creating a directory
    return 0;
}

int vsfs_open_file(const char* path, file_mode mode) {
    if (the_disk == NULL) {
        fprintf(stderr, "ERROR [%s]: Filesystem has not been initialized.\n", __func__);
        return -1;
    }
    if (path == NULL || path[0] != '/') {
        fprintf(stderr, "ERROR [%s]: Invalid path.", __func__);
        return -1;
    }
    inum inode_num = vsfs_traverse_to_inode(path);
    if (inode_num == INVALID_INODE) {
        fprintf(stderr, "ERROR [%s]: Path '%s' does not exist\n", __func__, path);
        return -1;
    }
    blocknum inode_table_block_num = superblock_snapshot.super.first_inode_block_num + (inode_num / INODES_PER_BLOCK);
    inum inode_index_in_block = inode_num % INODES_PER_BLOCK;
    metadata_block inode_table_block;
    int status = the_disk->read_block(the_disk, inode_table_block_num, (disk_block*)&inode_table_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to read inode block %u from the disk.\n", __func__, inode_table_block_num);
        return -1;
    }
    if (inode_table_block.inode_table.inodes[inode_index_in_block].type != FILE_TYPE_REGULAR) {
        fprintf(stderr, "ERROR [%s]: Path '%s' is not a regular file.\n", __func__, path);
        return -1;
    }
    int fd = vsfs_new_file_descriptor(inode_num, mode);
    if (fd < 0) {
        fprintf(stderr, "ERROR [%s]: Failed to allocate a new file descriptor.\n", __func__);
        return -1;
    }
    return fd;
}

signed_size vsfs_read_file(int fd, void* buffer, size_t size) {
    if (the_disk == NULL) {
        fprintf(stderr, "ERROR [%s]: Filesystem has not been initialized.\n", __func__);
        return -1;
    }
    if (fd < 0 || buffer == NULL) {
        fprintf(stderr, "ERROR [%s]: Invalid argument.\n", __func__);
        return -1;
    }
    // Look up the file table entry using the file descriptor
    file_table_entry* file_entry;
    void* file_entry_void;
    int status = vector_get(file_table, fd, &file_entry_void);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: File descriptor %d is invalid (not in the file table).\n", __func__, fd);
        return -1;
    }
    file_entry = file_entry_void;
    if (file_entry->mode == FILE_MODE_WRITE) {
        fprintf(stderr, "ERROR [%s]: File was opened in write-only mode, cannot read.\n", __func__);
        return -1;
    }
    // Retrieve the inode for this file
    int inode_table_block_num = superblock_snapshot.super.first_inode_block_num + (file_entry->inode_num / INODES_PER_BLOCK);
    inum inode_offset = file_entry->inode_num % INODES_PER_BLOCK;
    metadata_block inode_table_block;
    status = the_disk->read_block(the_disk, inode_table_block_num, (disk_block*)&inode_table_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to read inode block %u from the disk.\n", __func__, inode_table_block_num);
        return -1;
    }
    inode* inode = &inode_table_block.inode_table.inodes[inode_offset];

    // Clamp the read size to the remaining bytes in the file, so we only read blocks that exist
    size_t read_target_size = min_size_t(size, inode->size - file_entry->read_offset);

    // Read read_target_size bytes, one block at a time, starting at read_offset
    size_t bytes_read = 0;
    disk_block current_block;
    metadata_block indirect_block;
    size_t current_block_index = file_entry->read_offset / BLOCK_SIZE;
    bool indirect_block_read = false;
    while (bytes_read < read_target_size) {
        // Translate the in-file block index into a block number
        blocknum current_block_num;
        if (current_block_index < NUM_DIRECT_BLOCKS) {
            current_block_num = superblock_snapshot.super.first_data_block_num
                + inode->direct_blocks[current_block_index];
        } else {
            if (!indirect_block_read) {
                status = the_disk->read_block(the_disk, superblock_snapshot.super.first_data_block_num
                    + inode->indirect_block, (disk_block*)&indirect_block);
                if (status != 0) {
                    fprintf(stderr, "ERROR [%s]: Failed to read file indirect block %u from the disk.\n", __func__, superblock_snapshot.super.first_data_block_num + inode->indirect_block);
                    return -1;
                }
            }
            current_block_num = superblock_snapshot.super.first_data_block_num
                + indirect_block.indirect.block_numbers[current_block_index - NUM_DIRECT_BLOCKS];
        }
        status = the_disk->read_block(the_disk, current_block_num, &current_block);
        if (status != 0) {
            fprintf(stderr, "ERROR [%s]: Failed to read file data block %u from the disk.\n", __func__, current_block_num);
            return -1;
        }
        // Read this block until either the target size or the end of the block
        size_t position_in_block = (file_entry->read_offset + bytes_read) % BLOCK_SIZE;
        size_t bytes_to_read = min_size_t(read_target_size - bytes_read, BLOCK_SIZE - position_in_block);
        memcpy((uint8_t*)buffer + bytes_read, &current_block.bytes[position_in_block], bytes_to_read);
        bytes_read += bytes_to_read;
        // If the loop repeats it's because we read the entire block, so advance to the next one
        current_block_index++;
    }
    // If the read completed successfully, advance the file table's offset
    file_entry->read_offset += (int)bytes_read;
    return bytes_read;
}

signed_size vsfs_write_file(int fd, const void* buffer, size_t size) {
    if (the_disk == NULL) {
        fprintf(stderr, "ERROR [%s]: Filesystem has not been initialized.\n", __func__);
        return -1;
    }
    if (fd < 0 || buffer == NULL) {
        fprintf(stderr, "ERROR [%s]: Invalid argument.\n", __func__);
        return -1;
    }
    // Look up the file table entry using the file descriptor
    file_table_entry* file_entry;
    void* file_entry_void;
    int status = vector_get(file_table, fd, &file_entry_void);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: File descriptor %d is invalid (not in the file table).\n", __func__, fd);
        return -1;
    }
    file_entry = file_entry_void;
    if (file_entry->mode == FILE_MODE_READ) {
        fprintf(stderr, "ERROR [%s]: File was opened in read-only mode, cannot write.\n", __func__);
        return -1;
    }
    // Retrieve the inode for this file
    blocknum inode_table_block_num = superblock_snapshot.super.first_inode_block_num + (file_entry->inode_num / INODES_PER_BLOCK);
    inum inode_offset = file_entry->inode_num % INODES_PER_BLOCK;
    metadata_block inode_table_block;
    status = the_disk->read_block(the_disk, inode_table_block_num, (disk_block*)&inode_table_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to read inode block %u from the disk.\n", __func__, inode_table_block_num);
        return -1;
    }
    inode* inode = &inode_table_block.inode_table.inodes[inode_offset];

    // Determine if the write will go past the direct blocks
    bool write_uses_indirect = (file_entry->write_offset + size) / BLOCK_SIZE + 1 > NUM_DIRECT_BLOCKS;
    // Load the indirect block if it exists and will be needed
    metadata_block indirect_block;
    if (write_uses_indirect && inode->indirect_block != 0) {
        status = the_disk->read_block(the_disk, superblock_snapshot.super.first_data_block_num
            + inode->indirect_block, (disk_block*)&indirect_block);
        if (status != 0) {
            fprintf(stderr, "ERROR [%s]: Failed to read indirect block %u from the disk.\n", __func__, superblock_snapshot.super.first_data_block_num + inode->indirect_block);
            return -1;
        }
    }

    // Determine if the write will need new blocks
    int size_after_write = max_int(file_entry->write_offset + size, inode->size);
    // File size 0 is a special case: it has 0 blocks, even though ceil(size / BLOCK_SIZE) is 1
    int current_size_blocks = inode->size == 0 ? 0 : inode->size / BLOCK_SIZE + 1;
    int new_size_blocks = size_after_write / BLOCK_SIZE + 1;
    int num_new_blocks = new_size_blocks - current_size_blocks;
    if (num_new_blocks > 0) {
        disk_block data_bitmap_block;
        status = the_disk->read_block(the_disk, superblock_snapshot.super.data_bitmap_block_num, &data_bitmap_block);
        if (status != 0) {
            fprintf(stderr, "ERROR [%s]: Failed to read the data bitmap block from the disk.\n", __func__);
            return -1;
        }
        if (write_uses_indirect && inode->indirect_block == INVALID_DATA_BLOCK) {
            // If the file didn't have an indirect block but needs one, allocate one now
            int indirect_block_num = bitmap_block_allocate(&data_bitmap_block);
            if (indirect_block_num < 0) {
                fprintf(stderr, "ERROR [%s]: Filesystem is out of free blocks.\n", __func__);
                return -1;
            }
            inode->indirect_block = indirect_block_num;
            // Initialize the existing indirect_block variable as a new empty indirect block
            for (size_t i = 0; i < REFS_PER_BLOCK; ++i) {
                indirect_block.indirect.block_numbers[i] = INVALID_DATA_BLOCK;
            }
        }
        // Allocate the new blocks and add their numbers to the inode
        for (int b = 0; b < num_new_blocks; ++b) {
            int block_index = current_size_blocks + b;
            int new_block_num = bitmap_block_allocate(&data_bitmap_block);
            if (new_block_num < 0) {
                fprintf(stderr, "ERROR [%s]: Filesystem is out of free blocks.\n", __func__);
                return -1;
            }
            if (block_index < NUM_DIRECT_BLOCKS) {
                inode->direct_blocks[block_index] = new_block_num;
            } else {
                indirect_block.indirect.block_numbers[block_index - NUM_DIRECT_BLOCKS] = new_block_num;
            }
        }
        // Write back the bitmap block and possibly indirect block (if new blocks were added to it)
        status = the_disk->write_block(the_disk, superblock_snapshot.super.data_bitmap_block_num, &data_bitmap_block);
        if (status != 0) {
            fprintf(stderr, "ERROR [%s]: Failed to write the data bitmap block to the disk.\n", __func__);
            return -1;
        }
        if (current_size_blocks + num_new_blocks > NUM_DIRECT_BLOCKS) {
            status = the_disk->write_block(the_disk, superblock_snapshot.super.first_data_block_num + inode->indirect_block, (disk_block*)&indirect_block);
            if (status != 0) {
                fprintf(stderr, "ERROR [%s]: Failed to write indirect block %u to the disk.\n", __func__, superblock_snapshot.super.first_data_block_num + inode->indirect_block);
                return -1;
            }
        }
    }

    // First, fill up the current block that write_offset is within
    // This requires reading the current data first, then doing a write that starts at write_offset
    int current_block_index = file_entry->write_offset / BLOCK_SIZE;
    // Translate the in-file block index into a block number
    blocknum current_block_num;
    if (current_block_index < NUM_DIRECT_BLOCKS) {
        current_block_num = superblock_snapshot.super.first_data_block_num
            + inode->direct_blocks[current_block_index];
    } else {
        current_block_num = superblock_snapshot.super.first_data_block_num
            + indirect_block.indirect.block_numbers[current_block_index - NUM_DIRECT_BLOCKS];
    }
    disk_block block_to_write;
    status = the_disk->read_block(the_disk, current_block_num, &block_to_write);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to read data block %u from the disk\n", __func__, current_block_num);
        return -1;
    }
    int current_block_offset = file_entry->write_offset % BLOCK_SIZE;
    // The partial-block write will either write the entire buffer or the distance to the end of the block
    size_t bytes_written = min_size_t(size, BLOCK_SIZE - current_block_offset);
    memcpy(&block_to_write.bytes[current_block_offset], buffer, bytes_written);
    status = the_disk->write_block(the_disk, current_block_num, &block_to_write);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to write back data block %u to the disk\n", __func__, current_block_num);
        return -1;
    }
    // If there are any remaining bytes to write, they will always start at the beginning of a block
    while (bytes_written < size) {
        current_block_index++;
        // Translate index into block number again
        if (current_block_index < NUM_DIRECT_BLOCKS) {
            current_block_num = superblock_snapshot.super.first_data_block_num
                + inode->direct_blocks[current_block_index];
        } else {
            current_block_num = superblock_snapshot.super.first_data_block_num
                + indirect_block.indirect.block_numbers[current_block_index - NUM_DIRECT_BLOCKS];
        }
        size_t bytes_to_write = min_size_t(size - bytes_written, BLOCK_SIZE);
        // If the remaining bytes to write is less than a block, read the current block before writing
        // (if bytes_to_write == BLOCK_SIZE, block_to_write gets completely overwritten so don't bother)
        if (bytes_to_write < BLOCK_SIZE) {
            status = the_disk->read_block(the_disk, current_block_num, &block_to_write);
            if (status != 0) {
                fprintf(stderr, "ERROR [%s]: Failed to read data block %u from the disk\n", __func__, current_block_num);
                return -1;
            }
        }
        memcpy(&block_to_write.bytes[0], (uint8_t*)buffer + bytes_written, bytes_to_write);
        bytes_written += bytes_to_write;
        status = the_disk->write_block(the_disk, current_block_num, &block_to_write);
        if (status != 0) {
            fprintf(stderr, "ERROR [%s]: Failed to write data block %u to the disk\n", __func__, current_block_num);
            return -1;
        }
    }
    // Update and write back the inode
    inode->size = size_after_write;
    status = the_disk->write_block(the_disk, inode_table_block_num, (disk_block*)&inode_table_block);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to write inode block %u to the disk.\n", __func__, inode_table_block_num);
        return -1;
    }
    // If the write completed successfully, advance the file table's offset
    file_entry->write_offset += (int) bytes_written;
    return bytes_written;
}

int vsfs_close_file(int fd) {
    if (the_disk == NULL) {
        fprintf(stderr, "ERROR [%s]: Filesystem has not been initialized.\n", __func__);
        return -1;
    }
    if (fd < 0) {
        fprintf(stderr, "ERROR [%s]: Invalid argument.\n", __func__);
        return -1;
    }
    void* entry;
    int status = vector_get(file_table, fd, &entry);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: File descriptor %d is invalid (not in the file table).\n", __func__, fd);
        return -1;
    }
    free(entry);
    status = vector_set(file_table, fd, NULL);
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to set table entry %d to NULL.\n", __func__, fd);
        return -1;
    }
    return 0;
}

int vsfs_shutdown() {
    while (vector_size(file_table) > 0) {
        void* entry;
        if (vector_pop_back(file_table, &entry) == 0) {
            if (entry != NULL) {
                free(entry);
            }
        }
    }
    vector_free(file_table);
    file_table = NULL;
    the_disk = NULL;
    return 0;
}

/**
 * Adds a new entry to the filesystem's open-file table and returns the
 * associated file descriptor.
 *
 * @param inode_num The inode number of the open file.
 * @param mode The access mode the file is open with.
 * @return A file descriptor representing this open file.
 */
int vsfs_new_file_descriptor(inum inode_num, file_mode mode) {
    int status;
    // The next file descriptor is either vector_size, if the vector will need to grow,
    // or the lowest-numbered empty vector entry, if there is an empty space
    int fd = vector_size(file_table);
    for (int i = 0; i < vector_size(file_table); ++i) {
        void* entry_i;
        status = vector_get(file_table, i, &entry_i);
        if (status != 0) {
            fprintf(stderr, "ERROR [%s]: Failed to get index %d from the file table.\n", __func__, i);
            return -1;
        }
        if (entry_i == NULL) {
            fd = i;
            break;
        }
    }
    file_table_entry* new_table_entry = checked_malloc(sizeof(file_table_entry));
    new_table_entry->file_descriptor = fd;
    new_table_entry->inode_num = inode_num;
    new_table_entry->mode = mode;
    new_table_entry->read_offset = 0;
    new_table_entry->write_offset = 0;
    if (fd < vector_size(file_table)) {
        status = vector_set(file_table, fd, new_table_entry);
    } else {
        status = vector_push_back(file_table, new_table_entry);
    }
    if (status != 0) {
        fprintf(stderr, "ERROR [%s]: Failed to put a new file_table_entry in the table.\n", __func__);
        free(new_table_entry);
        return -1;
    }
    return fd;
}

/**
 * Looks up the inode number corresponding to an absolute path, traversing
 * directories from the root of the filesystem as necessary.
 *
 * @param path Pointer to a null-terminated string containing a path
 * @return The inode number of the last element (file or directory) in the path,
 * or INVALID_INODE if there was an error and the inode could not be found.
 */
inum vsfs_traverse_to_inode(const char* path) {
    if (path == NULL || strlen(path) == 0 || path[0] != '/') {
        return INVALID_INODE;
    }
    // Check for root directory
    if (strcmp(path, "/") == 0) {
        return ROOT_INODE;
    }
    int num_substrings = 0;
    char** path_elements = string_split(path, "/", &num_substrings);
    // current_inode_num is the inode of the *parent* of path element i
    inum current_inode_num = ROOT_INODE;
    // The first array element will be "" because the path starts with "/"
    for (int element_i = 1; element_i < num_substrings; ++element_i) {
        // Compute the block and offset of current_inode_num
        blocknum inode_table_block_num = superblock_snapshot.super.first_inode_block_num
            + (current_inode_num / INODES_PER_BLOCK);
        inum inode_index_in_block = current_inode_num % INODES_PER_BLOCK;
        // Read the block containing the inode and get a pointer to the inode
        metadata_block inode_table_block;
        the_disk->read_block(the_disk, inode_table_block_num, (disk_block*)&inode_table_block);
        inode* current_inode = &inode_table_block.inode_table.inodes[inode_index_in_block];
        if (current_inode->type != FILE_TYPE_DIRECTORY) {
            fprintf(stderr, "ERROR [%s]: Intermediate inode %d in path %s was not a directory\n", __func__, current_inode_num, path);
            for (int x = 0; x < num_substrings; ++x) free(path_elements[x]);
            free(path_elements);
            return INVALID_INODE;
        }
        // Get the directory data block for the current inode
        metadata_block directory_block;
        the_disk->read_block(the_disk, superblock_snapshot.super.first_data_block_num
            + current_inode->direct_blocks[0], (disk_block*)&directory_block);
        // Search for the next path element (element i) in this directory
        bool found = false;
        for (size_t i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; ++i) {
            if (strcmp(directory_block.directory.entries[i].name, path_elements[element_i]) == 0) {
                // This advances current_inode_num, right before the next loop iteration
                current_inode_num = directory_block.directory.entries[i].inode_num;
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "ERROR [%s]: Could not find element %s of path %s in its parent directory\n", __func__, path_elements[element_i], path);
            for (int x = 0; x < num_substrings; ++x) free(path_elements[x]);
            free(path_elements);
            return INVALID_INODE;
        }
    }
    // When the loop ends current_inode_num will be the inode number of the last path element
    for (int x = 0; x < num_substrings; ++x) free(path_elements[x]);
    free(path_elements);
    return current_inode_num;
}
