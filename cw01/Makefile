CC = gcc
CFLAGS = -Wall -fPIC $(OPTIMIZE)
OBJECTS = static_array.o dynamic_array.o timing.o

libarrays.a: $(OBJECTS)
		ar rcs $@ $^

libarrays.so: $(OBJECTS)
		$(CC) $(CFLAGS) -shared -o $@ $^

test-static: libarrays.a test.c
		$(CC) $(CFLAGS) -o $@ test.c -L. -larrays

test-shared: libarrays.so test.c
		$(CC) $(CFLAGS) -o $@ test.c -L. -larrays

test-loaded: libarrays.so test-loaded.c
		$(CC) $(CFLAGS) -o $@ test-loaded.c -ldl

clean:
		rm -f *.o *.a *.so test-static test-shared test-loaded