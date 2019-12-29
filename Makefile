CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -pedantic -O3
LDFLAGS = -lm $(shell pkg-config --libs sdl2)
GL_LDFLAGS = $(shell pkg-config --libs gl glu glew)
INCLUDES = $(shell pkg-config --cflags sdl2)
GL_INCLUDES = $(shell pkg-config --cflags gl glu glew)

.PHONY: default
default: color_cycling_plasma rgb_plasma gl_rgb_plasma 3d_plasma

color_cycling_plasma: src/color_cycling_plasma.c
	$(CC) src/color_cycling_plasma.c -o color_cycling_plasma $(CFLAGS) $(LDFLAGS) $(INCLUDES)

rgb_plasma: src/rgb_plasma.c
	$(CC) src/rgb_plasma.c -o rgb_plasma $(CFLAGS) $(LDFLAGS) $(INCLUDES)

gl_rgb_plasma: src/gl_rgb_plasma.c
	$(CC) src/gl_rgb_plasma.c -o gl_rgb_plasma $(CFLAGS) $(LDFLAGS) $(GL_LDFLAGS) $(INCLUDES) $(GL_INCLUDES)

3d_plasma: src/3d_plasma.c
	$(CC) src/3d_plasma.c -o 3d_plasma $(CFLAGS) $(LDFLAGS) $(GL_LDFLAGS) $(INCLUDES) $(GL_INCLUDES)

.PHONY: format
format:
	clang-format --verbose -i -style=file src/*.c src/*.h

.PHONY: clean
clean:
	rm -f color_cycling_plasma rgb_plasma gl_rgb_plasma
	rm -f **/*.o
	rm -rf *.dSYM
