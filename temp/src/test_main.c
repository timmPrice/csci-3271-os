#include "disk.h"
#include "fs.h"
#include "util/random.h"
#include "util/checked_malloc.h"
#include "util/signed_size.h"
#include "util/string_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

bool test_write_read(const char* filename, size_t data_size) {
    char* test_data = checked_malloc(data_size);
    random_string(test_data, data_size);
    int fd_write = vsfs_open_file(filename, FILE_MODE_WRITE);
    if (fd_write < 0) {
        free(test_data);
        return false;
    }
    signed_size bytes_written = vsfs_write_file(fd_write, test_data, data_size);
    bool success = (bytes_written == data_size);
    int status = vsfs_close_file(fd_write);
    success = success && (status == 0);
    int fd_read = vsfs_open_file(filename, FILE_MODE_READ);
    if (fd_read < 0) {
        free(test_data);
        return false;
    }
    char* read_result = checked_malloc(data_size);
    signed_size bytes_read = vsfs_read_file(fd_read, read_result, data_size);
    success = success && (bytes_read == data_size);
    int compare = strcmp(test_data, read_result);
    success = success && (compare == 0);
    free(test_data);
    free(read_result);
    return success;
}

int main() {
    srand((unsigned int)time(NULL));

    disk_interface* testing_disk = memory_disk_new(64);
    vsfs_init(testing_disk, 80);

    // Create some directories and files
    int status = vsfs_mkdir("/home");
    if (status != 0) {
        printf("Failed to create /home\n");
        return -1;
    }
    status = vsfs_mkdir("/home/myuser");
    if (status != 0) {
        printf("Failed to create /home/myuser\n");
        return -1;
    }
    status = vsfs_mkdir("/home/myuser/documents");
    if (status != 0) {
        printf("Failed to create /home/myuser/documents\n");
        return -1;
    }
    int test1fd = vsfs_create("/home/myuser/documents/testfile1", FILE_MODE_WRITE);
    if (test1fd < 0) {
        printf("Failed to create /home/myuser/documents/testfile1\n");
        return -1;
    }
    int test2fd = vsfs_create("/home/myuser/documents/testfile2", FILE_MODE_WRITE);
    if (test2fd < 0) {
        printf("Failed to create /home/myuser/documents/testfile2\n");
        return -1;
    }
    printf("Successfully created directories and files.\n");

    // Close the received file descriptors
    status = vsfs_close_file(test1fd);
    if (status != 0) {
        printf("Close(test1fd) failed\n");
        return -1;
    }
    status = vsfs_close_file(test2fd);
    if (status != 0) {
        printf("Close(test2fd) failed\n");
        return -1;
    }

    // Test writing data to each file and reading it back
    bool test1 = test_write_read("/home/myuser/documents/testfile1", 100);
    if (test1) {
        printf("Write-read test on testfile1 succeeded!\n");
    } else {
        printf("Write-read test on testfile1 failed.\n");
        return -1;
    }
    bool test2 = test_write_read("/home/myuser/documents/testfile2", 15 * BLOCK_SIZE);
    if (test2) {
        printf("Write-read test on testfile2 succeeded!\n");
    } else {
        printf("Write-read test on testfile2 failed!.n");
        return -1;
    }

    status = vsfs_link("/home/myuser/documents/testfile1", "/home/myuser/link1");
    if (status != 0) {
        printf("Link failed\n");
        return -1;
    }
    printf("Link succeeded.\n");

    status = vsfs_unlink("/home/myuser/documents/testfile1");
    int linkfd = vsfs_open_file("/home/myuser/link1", FILE_MODE_READ);
    if (status != 0 || linkfd < 0) {
        printf("Unlink of original failed or link stopped working\n");
        return -1;
    }
    vsfs_close_file(linkfd);
    printf("Unlink of original succeeded, link still opens.\n");

    status = vsfs_unlink("/home/myuser/link1");
    if (status != 0 || vsfs_open_file("/home/myuser/link1", FILE_MODE_READ) >= 0) {
        printf("Unlink of last link failed\n");
        return -1;
    }
    printf("Unlink of last link succeeded, file deleted.\n");

    if (vsfs_link("/home/myuser", "/home/dirlink") == 0) {
        printf("Linking a directory should have failed\n");
        return -1;
    }
    printf("Linking a directory correctly failed.\n");

    return 0;
}
