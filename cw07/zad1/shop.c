#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>

#include "shop.h"

int get_shop_key(){

    // get home path
    char* home = getenv("HOME");

    // get shop semaphore key
    int key = ftok(home, SHOP_KEY_NUMBER);
    if(key == -1){
        perror("An error occurred while creating shop key");
        exit(1);
    }

    return key;
}
