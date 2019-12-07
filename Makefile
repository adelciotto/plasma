CC := gcc
CFLAGS := -Wall -Wextra -std=c99 -pedantic -g
LDFLAGS := -lSDL2
INCLUDES := $(pkg-config --cflags --libs sdl2)

plasma: plasma.o
	$(CC) plasma.o -o plasma $(CFLAGS) $(LDFLAGS) $(INCLUDES)

plasma.o: plasma.c
	$(CC) -c plasma.c -o plasma.o

.PHONY: clean
clean:
	rm -f plasma
	rm -f *.o
	rm -rf *.dSYM
