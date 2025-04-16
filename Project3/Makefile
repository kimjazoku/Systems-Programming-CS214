CC = gcc
CFLAGS = -fdiagnostics-color=always -g
TARGETS = mysh
all: $(TARGETS)

memgrind: mysh.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.0 $(TARGETS)
