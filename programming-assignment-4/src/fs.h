#ifndef __FS_H__
#define __FS_H__

#include "disk.h"
#include "util/signed_size.h"
#include <stdint.h>
#include <stddef.h>

#define MAX_FILE_NAME_LENGTH 32
#define NUM_DIRECT_BLOCKS 10
#define VSFS_MAGIC_NUMBER 0x53465356

/**
 * The type of a file, stored in its inode. FILE_TYPE_FREE means the inode is
 * actually free and does not refer to a file.
 */
typedef enum file_type {
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_FREE
} file_type;

/**
 * The access mode (read or write) a file is opened with.
 */
typedef enum file_mode {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_READWRITE
} file_mode;

/**
 * Type alias for inode numbers, which are unsigned integers
 */
typedef uint32_t inum;


/**
 * Definition of an inode in this filesystem. For any inode other than the root
 * directory, a direct or indirect block number of 0 indicates that the block
 * is not used; only the root directory can use data block 0. All block numbers
 * are logical block numbers starting at the first block of the data segment of
 * the disk, which starts at physical block number first_data_block_num recorded
 * in the superblock. Thus, to access any block using the number stored in an
 * inode, you must offset the number by first_data_block_num to get the physical
 * block number used by the underlying disk.
 */
typedef struct inode {
    file_type type;
    int size;
    int link_count;
    blocknum direct_blocks[NUM_DIRECT_BLOCKS];
    blocknum indirect_block;
} inode;

/**
 * Directories are arrays of these fixed-size directory entries. File names
 * have a short maximum length, and every entry reserves space for the maximum
 * name length.
 */
typedef struct directory_entry {
    inum inode_num;
    char name[MAX_FILE_NAME_LENGTH];
} directory_entry;

/**
 * The filesystem keeps a table of open files in memory while running. Each
 * entry in the table is this structure; the file descriptor is also the index
 * of the entry in the table.
 */
typedef struct file_table_entry {
    int file_descriptor;
    inum inode_num;
    file_mode mode;
    int read_offset;
    int write_offset;
} file_table_entry;

/* Constants derived from the block size and data structure sizes */
#define INODES_PER_BLOCK (BLOCK_SIZE / sizeof(inode))
#define DIRECTORY_ENTRIES_PER_BLOCK (BLOCK_SIZE / sizeof(directory_entry))
#define REFS_PER_BLOCK (BLOCK_SIZE / sizeof(blocknum))

/**
 * Data layout of the superblock. This will be stored in the disk's first block.
 * The superblock defines where the disk is separated into metadata and data
 * segments: blocks lower than first_data_block_num are used for metadata, and
 * first_data_block_num and up are used for data.
 */
typedef struct superblock {
    int magic_number;
    blocknum inode_bitmap_block_num;
    blocknum data_bitmap_block_num;
    blocknum first_inode_block_num;
    blocknum first_data_block_num;
    int num_data_blocks;
} superblock;

/**
 * Data layout of an inode table block.
 */
typedef struct inode_table_block {
    inode inodes[INODES_PER_BLOCK];
} inode_table_block;

/**
 * Data layout of a directory-data block.
 */
typedef struct directory_block {
    directory_entry entries[DIRECTORY_ENTRIES_PER_BLOCK];
} directory_block;

/**
 * Data layout of an indirect block.
 */
typedef struct indirect_block {
    blocknum block_numbers[REFS_PER_BLOCK];
} indirect_block;

/**
 * A data type that represents a metadata block in the file system. The
 * disk_block member ensures the union is exactly the same size as a disk block
 * (which may not be true of the other block types if BLOCK_SIZE is not an
 * exact multiple of their data field sizes), while the block-type members
 * allow the union to be interpreted as any of the metadata block types. A
 * pointer to a disk_block can be cast to a pointer to a metadata_block to
 * interpret the disk block's contents as one of these metadata block types.
 */
typedef union metadata_block {
    disk_block block_data;
    superblock super;
    inode_table_block inode_table;
    directory_block directory;
    indirect_block indirect;
} metadata_block;

/**
 * Initializes a new filesystem on the specified disk. This creates a new
 * superblock, bitmap blocks, inode table, and empty root directory. It saves
 * the disk pointer to the filesystem's in-memory state, to be used on
 * subsequent calls to filesystem functions.
 *
 * @param disk A pointer to the disk object to use for filesystem operations.
 * @param num_inodes The number of inodes to initialize this filesystem with.
 * @return 0 if the filesystem was created successfully, -1 if there was an error.
 */
int vsfs_init(disk_interface* disk, int num_inodes);

/**
 * Loads an existing VSFS filesystem from the specified disk. This saves the
 * disk pointer to the filesystem's in-memory state, to be used on subsequent
 * calls to filesystem functions.
 *
 * @param disk A pointer to the disk object to use for filesystem operations.
 * @return 0 if the filesystem was loaded successfully, -1 if there was an error.
 */
int vsfs_load(disk_interface* disk);

/**
 * Creates a new empty file at the specified path and opens it in the specified
 * mode. The path must be absolute (starting with "/"); relative paths are not
 * supported. The parent directory must already exist.
 *
 * @param path The path to the file to create
 * @param mode The access mode to open the new file in
 * @return The file descriptor of the new file, or -1 if there was an error.
 */
int vsfs_create(const char* path, file_mode mode);

/**
 * Creates a new empty directory at the specified path. The path must be
 * absolute (starting with "/"); relative paths are not supported. If the
 * directory already exists, or the parent directory does not eixist, this
 * operation will fail.
 *
 * @param path The path to the directory to create
 * @return 0 if the operation succeeded, -1 if there was an error.
 */
int vsfs_mkdir(const char* path);

/**
 * Creates a new link to an existing file. This adds a directory entry to the
 * parent of dest_path, pointing to the same file as named in source_path. If
 * the parent of dest_path does not exist, or source_path does not exist, this
 * operation will fail.
 *
 * @param source_path The path to an existing file to link to
 * @param dest_path The path to the new filename to create. All parent elements
 * of the path must already exist; only the (base) file name will be created.
 * @return 0 if successful, -1 if there was an error.
 */
int vsfs_link(const char* source_path, const char* dest_path);

/**
 * Removes a link to a file (deletes a directory entry). If this removes the
 * last link to the file, it also deletes the file.
 *
 * @param path The path to a file to unlink
 * @return 0 if successful, -1 if there was an error.
 */
int vsfs_unlink(const char* path);

/**
 * Opens an existing regular file for reading, writing, or both. The file must
 * already exist (create it with vsfs_create first). The path to the file must
 * be absolute (starting with "/"); relative paths are not supported.
 *
 * @param path The path to the file to open.
 * @param mode A file_mode value specifying the access mode to open in
 * @return A file descriptor if the operation succeeded, -1 if it failed
 */
int vsfs_open_file(const char* path, file_mode mode);

/**
 * Attempts to read up to size bytes of data from an open file into a buffer.
 *
 * @param fd The file descriptor indicating the file to read
 * @param buffer A byte buffer to read into; must be at least size bytes.
 * @param size The number of bytes to read from the file.
 * @return The number of bytes successfully read, or -1 if there was an error.
 */
signed_size vsfs_read_file(int fd, void* buffer, size_t size);

/**
 * Attempts to write up to size bytes of data from a buffer into an open file.
 *
 * @param fd The file descriptor indicating the file to write.
 * @param buffer A byte buffer containing some data to write; must be at least size bytes.
 * @param size The number of bytes to write.
 * @return The number of bytes successfully written, or -1 if there was an error.
 */
signed_size vsfs_write_file(int fd, const void* buffer, size_t size);

/**
 * Closes an open file.
 *
 * @param fd The file descriptor indicating the file to close.
 * @return 0 if the operation succeeded, -1 if it failed.
 */
int vsfs_close_file(int fd);

/**
 * Closes all open files and deletes the in-memory state of the filesystem.
 *
 * @return 0 if the operation succeeded, -1 if it failed.
 */
int vsfs_shutdown();

#endif
