CC = gcc
CFLAGS = -Wall -std=gnu11

barber: barber.c shop.c
	$(CC) $(CFLAGS) $^ -o $@ -lrt -lpthread

customers: customers.c shop.c
	$(CC) $(CFLAGS) $^ -o $@ -lrt -lpthread

all: barber customers

clean:
	rm -f *.o barber customers