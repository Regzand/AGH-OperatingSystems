#include <string.h>
#include <stdlib.h>

#include "dynamic_array.h"

// initializes block array structure
void init_blockarray_dynamic(blockarray_dynamic* array, int size){
    array -> size = size;
    array -> block_sizes = calloc(size, sizeof(int));
    array -> blocks = calloc(size, sizeof(char*));
}

// frees block array structure
void free_blockarray_dynamic(blockarray_dynamic* array){
    for(int i = 0; i < array -> size; i++)
        free_block_dynamic(array, i);

    free(array -> block_sizes);
    array -> block_sizes = NULL;
    free(array -> blocks);
    array -> blocks = NULL;
}

// sets block at given index
void add_block_dynamic(blockarray_dynamic* array, int index, char* block, int block_size){
    array -> block_sizes[index] = block_size;
    array -> blocks[index] = realloc(array -> blocks[index], sizeof(char) * block_size);
    strcpy(array -> blocks[index], block);
}

// frees the block at given index
void free_block_dynamic(blockarray_dynamic* array, int index){
    array -> block_sizes[index] = 0;
    free(array -> blocks[index]);
    array -> blocks[index] = NULL;
}

// sums data in block
int sum_block_dynamic(char* block, int block_size){
    int sum = 0;
    for(int i = 0; i < block_size; i++)
        sum += (int) block[i];
    return sum;
}

// finds index of a block that is the closest (according to sum of ASCII chars) to the given one
int find_block_dynamic(blockarray_dynamic* array, int index){
    // get target value
    int value = sum_block_dynamic(array -> blocks[index], array -> block_sizes[index]);

    // closest block
    int ans_id = -1;
    int ans_diff = -1;

    for(int i = 0; i < array -> size; i++){
        // skip target block since diff is always 0
        if(i == index) continue;

        // get block value
        int block_value = sum_block_dynamic(array -> blocks[i], array -> block_sizes[i]);

        // if this is closer set it as the closest block
        if(ans_diff > abs(value - block_value)){
            ans_diff = abs(value - block_value);
            ans_id = i;
        }
    }

    return ans_id;
}
