
#include <stdlib.h>

int main(int argc, char* args[]){
    char* buffer = calloc( ((long long int) atoi(args[1])) << 20, sizeof(char) );
    buffer[0] = '\0';
}
