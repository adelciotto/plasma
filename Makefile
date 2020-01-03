CFLAGS := -Wall -Wextra -Werror -std=c11 -pedantic -O3
LDFLAGS := -lm $(shell pkg-config --libs sdl2)
INCLUDES := $(shell pkg-config --cflags sdl2)

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux)
	GL_LDFLAGS := $(shell pkg-config --libs gl glew)
	GL_INCLUDES := $(shell pkg-config --cflags gl glew)
endif

ifeq ($(UNAME_S), Darwin)
	GL_LDFLAGS := $(shell pkg-config --libs glew) -framework OpenGL
	GL_INCLUDES := $(shell pkg-config --cflags glew)
endif


.PHONY: default
default: palette_plasma rgb_plasma gl_rgb_plasma cube_plasma

palette_plasma: src/palette_plasma.c
	$(CC) src/palette_plasma.c -o palette_plasma $(CFLAGS) $(LDFLAGS) $(INCLUDES)

rgb_plasma: src/rgb_plasma.c
	$(CC) src/rgb_plasma.c -o rgb_plasma $(CFLAGS) $(LDFLAGS) $(INCLUDES)

gl_rgb_plasma: src/gl_rgb_plasma.c src/glmath.h
	$(CC) src/gl_rgb_plasma.c -o gl_rgb_plasma $(CFLAGS) $(LDFLAGS) $(GL_LDFLAGS) $(INCLUDES) $(GL_INCLUDES)

cube_plasma: src/cube_plasma.c src/glmath.h
	$(CC) src/cube_plasma.c -o cube_plasma $(CFLAGS) $(LDFLAGS) $(GL_LDFLAGS) $(INCLUDES) $(GL_INCLUDES)

.PHONY: format
format:
	clang-format --verbose -i -style=file src/*.c src/*.h

.PHONY: clean
clean:
	rm -f palette_plasma rgb_plasma gl_rgb_plasma cube_plasma
	rm -f **/*.o
	rm -rf *.dSYM
