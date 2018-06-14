#include <stdio.h>
#include <stdlib.h>

void error(const char* msg) {
    perror(msg);
    exit(1);
}

void error_msg(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}
