CC = gcc
CFLAGS = -Wall -std=gnu11 -DLOG_USE_COLOR
LDLIBS = -lpthread -lm
LIBS = common.c ../utils/log.c

server: server.c ${LIBS}

client: client.c ${LIBS}

all: server client

clean:
	rm -f *.o server client