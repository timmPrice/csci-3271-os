#include "disk.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DEFAULT_NUM_BLOCKS 64

typedef struct ram_disk {
    disk_block* blocks;
    blocknum num_blocks;
} ram_disk;

int memory_write_block(disk_interface* self, blocknum block_num, const disk_block* block) {
    if (self == NULL || block == NULL || block_num >= ((ram_disk*)self->state)->num_blocks) {
        return -1;
    }
    ram_disk* disk_state = self->state;
    memcpy(disk_state->blocks[block_num].bytes, block->bytes, BLOCK_SIZE);
    return 0;
}

int memory_read_block(disk_interface* self, blocknum block_num, disk_block* block) {
    if (self == NULL || block == NULL || block_num >= ((ram_disk*)self->state)->num_blocks) {
        return -1;
    }
    ram_disk* disk_state = self->state;
    memcpy(block->bytes, disk_state->blocks[block_num].bytes, BLOCK_SIZE);
    return 0;
}

int memory_disk_size(disk_interface* self) {
    if (self == NULL) {
        return -1;
    }
    return ((ram_disk*)self->state)->num_blocks;
}

int memory_disk_shutdown(disk_interface* self) {
    if (self == NULL) {
        return -1;
    }
    free(((ram_disk*)self->state)->blocks);
    free(self->state);
    free(self);
    return 0;
}

disk_interface* memory_disk_new(blocknum desired_size) {
    disk_interface* interface = malloc(sizeof(disk_interface));
    if (interface == NULL) {
        return NULL;
    }
    ram_disk* disk_state = malloc(sizeof(ram_disk));
    if (disk_state == NULL) {
        free(interface);
        return NULL;
    }
    int disk_size = (desired_size > 0) ? desired_size : DEFAULT_NUM_BLOCKS;
    disk_state->num_blocks = disk_size;
    disk_state->blocks = malloc(disk_size * sizeof(disk_block));
    if (disk_state->blocks == NULL) {
        free(interface);
        free(disk_state);
        return NULL;
    }
    interface->state = disk_state;
    interface->read_block = memory_read_block;
    interface->write_block = memory_write_block;
    interface->get_size = memory_disk_size;
    interface->shutdown = memory_disk_shutdown;
    return interface;
}

typedef struct file_disk {
    FILE* file;
    blocknum num_blocks;
} file_disk;

int file_read_block(disk_interface* self, blocknum block_num, disk_block* block) {
    if (self == NULL || block == NULL || block_num >= ((file_disk*)self->state)->num_blocks) {
        return -1;
    }
    file_disk* disk_state = self->state;
    fseek(disk_state->file, block_num * BLOCK_SIZE, SEEK_SET);
    size_t result = fread(block->bytes, BLOCK_SIZE, 1, disk_state->file);
    if (result != 1) {
        return -1;
    }
    return 0;
}

int file_write_block(disk_interface* self, blocknum block_num, const disk_block* block) {
    if (self == NULL || block == NULL || block_num >= ((file_disk*)self->state)->num_blocks) {
        return -1;
    }
    file_disk* disk_state = self->state;
    fseek(disk_state->file, block_num * BLOCK_SIZE, SEEK_SET);
    size_t result = fwrite(block->bytes, BLOCK_SIZE, 1, disk_state->file);
    if (result != 1) {
        return -1;
    }
    return 0;
}

int file_disk_size(disk_interface* self) {
    if (self == NULL) {
        return -1;
    }
    return ((file_disk*)self->state)->num_blocks;
}

int file_disk_shutdown(disk_interface* self) {
    if (self == NULL) {
        return -1;
    }
    file_disk* disk_state = self->state;
    fclose(disk_state->file);
    free(self->state);
    free(self);
    return 0;
}

disk_interface* file_disk_new(blocknum desired_size, const char* filename) {
    disk_interface* interface = malloc(sizeof(disk_interface));
    if (interface == NULL) {
        return NULL;
    }
    file_disk* disk_state = malloc(sizeof(file_disk));
    if (disk_state == NULL) {
        free(interface);
        return NULL;
    }
    int disk_size = (desired_size > 0) ? desired_size : DEFAULT_NUM_BLOCKS;
    disk_state->num_blocks = disk_size;
    // Open in "append extended" mode to not delete existing data
    disk_state->file = fopen(filename, "a+");
    if(disk_state->file == 0) {
        perror(filename);
        return NULL;
    }
    // Seek to the end of the last block and write a null byte to ensure the file is the right size
    fseek(disk_state->file, disk_size * BLOCK_SIZE - 1, SEEK_SET);
    fwrite("", 1, 1, disk_state->file);
    fflush(disk_state->file);

    interface->state = disk_state;
    interface->read_block = file_read_block;
    interface->write_block = file_write_block;
    interface->get_size = file_disk_size;
    interface->shutdown = file_disk_shutdown;
    return interface;
}
