#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  // TODO: Make these configurable
  const int width = 640;
  const int height = 480;

  SDL_Window *window =
      SDL_CreateWindow("Plasma", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (window == NULL) {
    fprintf(stderr, "SDL_CreateWindow error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
    fprintf(stderr, "SDL_CreateRenderer error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_RenderSetLogicalSize(renderer, width, height);

  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32,
                                                        SDL_PIXELFORMAT_RGBA32);
  if (surface == NULL) {
    fprintf(stderr, "SDL_CreateRGBSurface error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                        SDL_TEXTUREACCESS_STREAMING, width, height);
  if (texture == NULL) {
    fprintf(stderr, "SDL_CreateTexture error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Event event;
  while (1) {
    SDL_PollEvent(&event);

    if (event.type == SDL_QUIT) {
      break;
    }

    if (SDL_LockSurface(surface) != 0) {
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
