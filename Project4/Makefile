CC = gcc
CFLAGS = -fdiagnostics-color=always -g
TARGETS = rpsd player.o
all: $(TARGETS)

rpsd: player.c rpsd.c
	$(CC) $(CFLAGS) $^ -o $@

player: player.c player.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.0 $(TARGETS)
