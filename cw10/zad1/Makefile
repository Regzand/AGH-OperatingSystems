CC = gcc
CFLAGS = -Wall -std=gnu11
LDLIBS = -lpthread -lm

server: server.c common.c

client: client.c common.c

all: server client

clean:
	rm -f *.o server client