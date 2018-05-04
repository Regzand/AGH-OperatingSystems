#ifndef SHOP_H
#define SHOP_H

// number used in creating shop key
#define SHOP_KEY_NUMBER  13

// needed for semaphore initialization
typedef union semun { int val; } semun;

// creates shop key (System V)
int get_shop_key();

#endif