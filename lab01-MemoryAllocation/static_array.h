#ifndef STATIC_ARRAY_H
#define STATIC_ARRAY_H

#define MAX_ARRAY_SIZE 1000
#define MAX_BLOCK_SIZE 1000

// block array structure
typedef struct blockarray_static {
    int  size;                                      // size of array
    int  block_sizes[MAX_ARRAY_SIZE];               // sizes of blocks
    char blocks[MAX_ARRAY_SIZE][MAX_BLOCK_SIZE];    // blocks
} blockarray_static;

// initializes block array structure
void init_blockarray_static(blockarray_static* array, int size);

// sets block at given index
void  add_block_static(blockarray_static* array, int index, char* block, int block_size);

// frees the block at given index
void free_block_static(blockarray_static* array, int index);

// finds index of a block that is the closest (according to sum of ASCII chars) to the given one
int  find_block_static(blockarray_static* array, int index);

#endif
