#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_TITLE "Plasma"

SDL_Window *CreateSDLWindow(int width, int height);
SDL_Renderer *CreateSDLRenderer(SDL_Window *window, int width, int height);
SDL_Surface *CreateSDLSurface(int width, int height);
SDL_Texture *CreateSDLTexture(SDL_Renderer *renderer, int width, int height);

int LockSDLSurface(SDL_Surface *surface);

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  // TODO: Make these configurable
  const int width = 640;
  const int height = 480;

  SDL_Window *window = CreateSDLWindow(width, height);
  if (window == NULL) {
    fprintf(stderr, "CreateSDLWindow error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Renderer *renderer = CreateSDLRenderer(window, width, height);
  if (renderer == NULL) {
    fprintf(stderr, "CreateSDLRenderer error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Surface *surface = CreateSDLSurface(width, height);
  if (surface == NULL) {
    fprintf(stderr, "CreateSDLSurface error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Texture *texture = CreateSDLTexture(renderer, width, height);
  if (texture == NULL) {
    fprintf(stderr, "CreateSDLTexture error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Event event;
  while (1) {
    SDL_PollEvent(&event);

    if (event.type == SDL_QUIT) {
      break;
    }

    if (LockSDLSurface(surface) != 0) {
      fprintf(stderr, "SDL_LockSurface error: %s", SDL_GetError());
      break;
    }

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        double r = x / (double)width;
        double g = y / (double)height;

        Uint32 *pixels = (Uint32 *)surface->pixels;
        int index = (y * surface->pitch / surface->format->BytesPerPixel) + x;
        pixels[index] = SDL_MapRGB(surface->format, (Uint8)(r * 255),
                                   (Uint8)(g * 255), 128);
      }
    }

    SDL_UnlockSurface(surface);

    SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
  }

  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}

SDL_Window *CreateSDLWindow(int width, int height) {
  return SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED, width, height,
                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
}

SDL_Renderer *CreateSDLRenderer(SDL_Window *window, int width, int height) {
  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  SDL_RenderSetLogicalSize(renderer, width, height);

  return renderer;
}

SDL_Surface *CreateSDLSurface(int width, int height) {
  return SDL_CreateRGBSurfaceWithFormat(0, width, height, 32,
                                        SDL_PIXELFORMAT_RGBA32);
}

SDL_Texture *CreateSDLTexture(SDL_Renderer *renderer, int width, int height) {
  return SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                           SDL_TEXTUREACCESS_STREAMING, width, height);
}

int LockSDLSurface(SDL_Surface *surface) {
  if (SDL_MUSTLOCK(surface)) {
    return SDL_LockSurface(surface);
  }

  return 0;
}
