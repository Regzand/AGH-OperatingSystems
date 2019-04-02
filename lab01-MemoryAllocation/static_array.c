#include <string.h>
#include <stdlib.h>

#include "static_array.h"

// initialize structure
void init_blockarray_static(blockarray_static* array, int size){
    array -> size = size;
    for(int i = 0; i < size; i++)
        array -> block_sizes[i] = 0;
}

// sets block at given index
void add_block_static(blockarray_static* array, int index, char* block, int block_size){
    array -> block_sizes[index] = block_size;
    strcpy(array -> blocks[index], block);
}

// free block at given index
void free_block_static(blockarray_static* array, int index){
    array -> block_sizes[index] = 0;
}

// sums data in block
int sum_block_static(char* block, int block_size){
    int sum = 0;
    for(int i = 0; i < block_size; i++)
        sum += (int) block[i];
    return sum;
}

// finds index of a block that is the closest (according to sum of ASCII chars) to the given one
int find_block_static(blockarray_static* array, int index){
    // get target value
    int value = sum_block_static(array -> blocks[index], array -> block_sizes[index]);

    // closest block
    int ans_id = -1;
    int ans_diff = -1;

    for(int i = 0; i < array -> size; i++){
        // skip target block since diff is always 0
        if(i == index) continue;

        // get block value
        int block_value = sum_block_static(array -> blocks[i], array -> block_sizes[i]);

        // if this is closer set it as the closest block
        if(ans_diff > abs(value - block_value)){
            ans_diff = abs(value - block_value);
            ans_id = i;
        }
    }

    return ans_id;
}
