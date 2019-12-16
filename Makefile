CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c99 -pedantic -O3
LDFLAGS := -lm -lSDL2
INCLUDES := $(pkg-config --cflags --libs sdl2)

.PHONY: default
default: color_cycling_plasma rgb_plasma

color_cycling_plasma: color_cycling_plasma.o
	$(CC) color_cycling_plasma.o -o color_cycling_plasma $(CFLAGS) $(LDFLAGS) $(INCLUDES)

rgb_plasma: rgb_plasma.o
	$(CC) rgb_plasma.o -o rgb_plasma $(CFLAGS) $(LDFLAGS) $(INCLUDES)

color_cycling_plasma.o: color_cycling_plasma.c
	$(CC) -c color_cycling_plasma.c -o color_cycling_plasma.o

rgb_plasma.o: rgb_plasma.c
	$(CC) -c rgb_plasma.c -o rgb_plasma.o

.PHONY: clean
clean:
	rm -f color_cycling_plasma rgb_plasma
	rm -f *.o
	rm -rf *.dSYM
