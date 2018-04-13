#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

// block array structure
typedef struct blockarray_dynamic {
    int size;               // size of array
    int* block_sizes;       // sizes of blocks
    char** blocks;          // blocks
} blockarray_dynamic;

// initializes block array structure
void init_blockarray_dynamic(blockarray_dynamic* array, int size);

// frees block array structure
void free_blockarray_dynamic(blockarray_dynamic* array);

// sets block at given index
void add_block_dynamic(blockarray_dynamic* array, int index, char* block, int block_size);

// frees the block at given index
void free_block_dynamic(blockarray_dynamic* array, int index);

// finds index of a block that is the closest (according to sum of ASCII chars) to the given one
int  find_block_dynamic(blockarray_dynamic* array, int index);

#endif
