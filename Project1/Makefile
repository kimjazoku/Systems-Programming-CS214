CC = gcc
CFLAGS = -g -Wall -std=c99 -fsanitize=address, undefined
TARGETS = memgrind mymalloc memtest
all: $(TARGETS)

memgrind: memgrind.c mymalloc.o
	$(CC) $(CFLAGS) $^ -o $@

mymalloc: mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -c $< -o $@

memtest: memtest.c mymalloc.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.0 $(TARGETS)

