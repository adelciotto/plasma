CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c99 -pedantic -O2
LDFLAGS := -lm -lSDL2
INCLUDES := $(pkg-config --cflags --libs sdl2)

color_cycling_plasma: color_cycling_plasma.o
	$(CC) color_cycling_plasma.o -o color_cycling_plasma $(CFLAGS) $(LDFLAGS) $(INCLUDES)

color_cycling_plasma.o: color_cycling_plasma.c
	$(CC) -c color_cycling_plasma.c -o color_cycling_plasma.o

.PHONY: clean
clean:
	rm -f color_cycling_plasma
	rm -f *.o
	rm -rf *.dSYM
