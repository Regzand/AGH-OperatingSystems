CC = gcc
CFLAGS = -Wall -std=gnu11

main: main.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lm

all: main

clean:
	rm -f *.o main