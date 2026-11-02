# Very Simple File System

This program implements a simple filesystem that can store data on any block device that provides the interface given in disk.h. It uses inodes to keep track of the location of file data, and inodes are stored in a table in a fixed location at the beginning of the block device. Each inode has 10 direct blocks and one indirect block. Directories are special files that have a single data block containing a table of directory entries, where each directory entry has room for an inode and a 32-character file name. The main function in test_main.c demonstrates how the file system is used by creating a few files and directories, writing data to the files, and reading the data back into memory.

## Compiling the program

This program is set up as a CMake project, which allows it to be compiled in either Windows or Linux as long as you have CMake installed.

On Windows, launch Visual Studio and choose the "Open Folder" option (rather than "Open Project"), then select the root vsfs-project folder that contains CMakeLists.txt and CMakePresets.json. You should then be able to choose the "Windows Debug" build configuration and build the project. The compiled executable will be placed in "out\build\windows-debug\src", and will be named vsfs_test.exe.

On Linux, open a terminal in the paging-simulation directory and run cmake with the linux-debug preset:

```console
~/vsfs-project$ cmake --preset linux-debug
```

This will create a directory named build-Debug, which you can then instruct CMake to compile in:

```console
~/vsfs-project$ cmake --build build-Debug
```

## Disk Objects

This program uses a form of polymorphism in C to represent the block device the filesystem uses to store data. The filesystem's initialization functions `vsfs_init` and `vsfs_load` receive a pointer to a `disk_interface`, which is a `struct` that contains several function pointers representing interface methods. The `main` function creates an instance of the `disk_interface` by calling one of the `disk_new` functions, which are (currently) `memory_disk_new` or `file_disk_new`. Each `new` function binds a different set of "method" functions to the `disk_interface`'s function pointers, giving the interface a specific implementation.

The disk interface is defined in disk.h, and the comments on each function pointer in the struct describe the intended functionality of each "method." The "method" functions, like other object-oriented C functions, always take as their first argument a pointer to the `disk_interface` object that called them, which is used like the `this` pointer or `self` reference in other languages. The disk interface also contains a `void` pointer named `state` that is intended to be used by each implementation of the interface to point to its instance variables; callers of the interface should not access `state` directly.

When VSFS is initialized (by calling `vsfs_init`), it saves the pointer to the `disk_interface` in a file-scope variable named `the_disk`, so it can be reused on all subsequent filesystem function calls. This means the filesystem can only use one disk object per execution of the program, but most programs will not create more than one filesystem, and this means other filesystem functions like `vsfs_open_file` do not need to pass a disk interface as a parameter. Whenever the filesystem reads or writes a block in a filesystem function, it invokes one of the disk methods using its saved pointer.

## File System Design Notes

VSFS stores a superblock in block 0 of the disk, containing important basic parameters of the filesystem: The location of the inode bitmap block, data bitmap block, and inode-table blocks, and the block number that begins the "data" section of the disk. This superblock is also stored in memory, in a file-scope variable named `superblock_snapshot`, since it frequently needs to be accessed and won't change after the file system is initialized.

The data blocks in this filesystem are numbered with **logical** block numbers, which start at 0 for the first data block, not the "real" physical block numbers used by the underlying disk. The free-block bitmap and the inodes use these logical block numbers, and the offset between logical block numbers and physical block numbers is the superblock field `first_data_block_num`. In other words, if `first_data_block_num` is 7, then an inode with a direct block entry of 2 is actually storing its data at disk block 7 + 2 = 9. You can see this in the implementation of `vsfs_read_file`, which computes the block number to use for a `read_block` call as

```c
current_block_num = superblock_snapshot.super.first_data_block_num + inode->direct_blocks[current_block_index];
```

The root directory of the filesystem is always stored using inode 1 and data block 0 (again, a logical block number, so it actually uses disk block `first_data_block_num`). Other directories can be stored in any inode and data block, but a directory can only use one data block; VSFS does not support extending directories to more than one data block. A directory's data block is a table of fixed-size directory entries, and an entry with an inode number of 0 is considered "empty" (inode number 0 is invalid).

Like ext2, VSFS uses exactly one block as a free-bitmap for data blocks and one block as a free-bitmap for inodes. Unlike ext2, VSFS does not support multiple block groups on a disk, so it is limited to disks of size 8 * BLOCK_SIZE blocks (the number of blocks that can be counted in a bitmap). Similarly, it can only have up to 8 * BLOCK_SIZE inodes in its inode table.

Files in this filesystem do not have owners, permissions, or last-modified timestamps.

## Utilities

The `src/util` directory contains some general-purpose data structures and functions that are used by the filesystem or the testing code in test_main.c. These include the `list` and `vector` data structures from the Paging Simulation project, and some free-standing string and integer functions. You may want to use some of these while writing your code. In particular, the `string_util` files contain functions for splitting a POSIX-style path (i.e. one that starts with "/" and separates directories with "/") into its different components. Note that the `path_parent` and `path_basename` functions expect the caller to allocate a character array for their output before calling them (similar to the standard library `strcpy` function), but the `string_split` function allocates its own two-dimensional array and returns it to the caller, who is then responsible for freeing the array later.