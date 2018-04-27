CC = gcc
CFLAGS = -Wall -std=gnu11

server: server.c settings.h
	$(CC) $(CFLAGS) server.c -o server -lrt

client: client.c settings.h
	$(CC) $(CFLAGS) client.c -o client -lrt

all: server client

clean:
	rm -f *.o server client