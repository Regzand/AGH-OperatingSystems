CC = gcc
CFLAGS = -Wall -std=gnu11

main: main.c utils/timing.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lm

clean:
	rm -f *.o main