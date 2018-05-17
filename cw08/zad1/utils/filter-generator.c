#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** args){
    srand(time(NULL));

    // check arguments count
    if (argc < 2) {
        fprintf(stderr, "Missing arguments!\nUsage: <filter size>\n");
        exit(1);
    }

    // parse arguments
    int C = atoi(args[1]);

    // create filter
    float* filter = malloc(sizeof(float)*C*C);

    // get random numbers
    float sum = 0;
    for(int i = 0; i<C*C; i++) {
        filter[i] = rand();
        sum += filter[i];
    }

    // normalize numbers so that they all add up to 1
    for(int i = 0; i<C*C; i++)
        filter[i] /= sum;

    // print header
    printf("%d\n", C);

    // print filter data
    for(int i = 0; i<C*C; i++)
        printf("%f ", filter[i]);
    printf("\n");

    return 0;
}