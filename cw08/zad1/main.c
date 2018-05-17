#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include "utils/timing.h"

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)

typedef struct batch {
    int from;
    int to;
} batch;

int threads_number; // number of threads to use

int M;              // max value of pixel

int *image;         // image data
int *output;        // created image data
int H;              // height of image
int W;              // width of image

float *filter;      // filter data
int C;              // size of
int center;         // center of filter, used in calculations

void calculate_pixel(int pixel) {
    int x = pixel % W;
    int y = pixel / W;

    float value = 0;

    for (int i = 0; i < C; ++i)
        for (int j = 0; j < C; ++j)
            value += image[min(W - 1, max(0, x + i - center)) + min(H - 1, max(0, y + j - center)) * W] *
                     filter[i + j * C];

    output[pixel] = (int) round(value);
}

void *calculate(void *args) {
    batch *b = args;

    for (int pixel = b->from; pixel < b->to; pixel++)
        calculate_pixel(pixel);

    return 0;
}

void save_output(char *file) {

    // open/create file
    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        perror("An error occurred while opening filter file");
        exit(1);
    }

    // print header
    fprintf(fp, "P2\n%d %d\n%d\n", W, H, M);

    // print image data
    for (int i = 0; i < H * W; i++)
        fprintf(fp, "%d ", output[i]);
    fprintf(fp, "\n");

    // close file
    if (fclose(fp) != 0) {
        perror("An error occurred while closing image file");
        exit(1);
    }
}

void load_image(char *file) {

    // open file
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        perror("An error occurred while opening image file");
        exit(1);
    }

    // skip 'P2' header, read height and width, and check coding
    fscanf(fp, "P2 %d %d %d", &W, &H, &M);
    if (M != 255) {
        fprintf(stderr, "Unexpected file coding %d, expected 255\n", M);
        exit(1);
    }

    // load file data
    image = malloc(H * W * sizeof(int));
    for (int i = 0; i < H * W; i++) {
        fscanf(fp, "%d", &image[i]);
    }

    // close file
    if (fclose(fp) != 0) {
        perror("An error occurred while closing image file");
        exit(1);
    }

    // logging
    printf("Loaded image %dx%d with coding %d\n", W, H, M);
}

void load_filter(char *file) {

    // open file
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        perror("An error occurred while opening filter file");
        exit(1);
    }

    // read filter size
    fscanf(fp, "%d", &C);

    // read filter data
    filter = malloc(C * C * sizeof(float));
    for (int i = 0; i < C * C; i++)
        fscanf(fp, "%f", &filter[i]);

    // close file
    if (fclose(fp) != 0) {
        perror("An error occurred while closing filter file");
        exit(1);
    }

    // calculate filter center
    center = (int) ceil(C / 2.0);

    // logging
    printf("Loaded filter %dx%d\n", C, C);
}

int main(int argc, char **args) {

    // check arguments count
    if (argc < 5) {
        fprintf(stderr, "Missing arguments!\nUsage: <threads> <img> <filter> <out>\n");
        exit(1);
    }

    // parse arguments
    threads_number = atoi(args[1]);

    // load data
    load_image(args[2]);
    load_filter(args[3]);

    // prepare output image storage
    output = malloc(H * W * sizeof(int));

    // prepare batches
    int batch_size = ((H * W) % threads_number == 0 ? (H * W) / threads_number : (H * W) / threads_number + 1);
    struct batch *batches = malloc(sizeof(struct batch) * threads_number);
    for (int i = 0; i < threads_number; i++) {
        batches[i].from = batch_size * i;
        batches[i].to = batch_size * (i + 1);
    }
    batches[threads_number - 1].to = H * W;

    // prepare timer
    struct timer T;

    // prepare threads
    pthread_t *tid = malloc(sizeof(pthread_t) * threads_number);

    // start timer
    start_timer(&T);

    // run threats
    for (int i = 0; i < threads_number; i++) {
        if (pthread_create(&tid[i], NULL, calculate, &batches[i]) != 0) {
            perror("An error occurred while creating thread");
            exit(1);
        }
    }

    // wait for threads to finish
    for (int i = 0; i < threads_number; i++) {
        if (pthread_join(tid[i], NULL) != 0) {
            perror("An error occurred while waiting for thread to finish");
            exit(1);
        }
    }

    // stop timer
    stop_timer(&T);

    // save output
    save_output(args[4]);

    // display info
    printf("Filtering using %d threads completed in:\n", threads_number);
    display_time(&T);

    return 0;
}