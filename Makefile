CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c99 -pedantic -O3
LDFLAGS := -lm -lSDL2
INCLUDES := $(pkg-config --cflags --libs sdl2)

.PHONY: default
default: color_cycling_plasma rgb_plasma

color_cycling_plasma: src/color_cycling_plasma.o
	$(CC) src/color_cycling_plasma.o -o color_cycling_plasma $(CFLAGS) $(LDFLAGS) $(INCLUDES)

rgb_plasma: src/rgb_plasma.o
	$(CC) src/rgb_plasma.o -o rgb_plasma $(CFLAGS) $(LDFLAGS) $(INCLUDES)

src/color_cycling_plasma.o: src/color_cycling_plasma.c
	$(CC) -c src/color_cycling_plasma.c -o src/color_cycling_plasma.o

src/rgb_plasma.o: src/rgb_plasma.c
	$(CC) -c src/rgb_plasma.c -o src/rgb_plasma.o

.PHONY: clean
clean:
	rm -f color_cycling_plasma rgb_plasma
	rm -f **/*.o
	rm -rf *.dSYM
