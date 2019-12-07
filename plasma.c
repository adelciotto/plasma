#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_TITLE "Plasma"

SDL_Window *CreateSDLWindow(int width, int height);
SDL_Renderer *CreateSDLRenderer(SDL_Window *window, int width, int height);
SDL_Surface *CreateSDLSurface(int width, int height);
SDL_Texture *CreateSDLTexture(SDL_Renderer *renderer, int width, int height);

int LockSDLSurface(SDL_Surface *surface);
void SetPixelSDLSurface(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g,
                        Uint8 b);

void DrawFrame(SDL_Surface *surface, double elapsedTime) {
  int width = surface->w;
  int height = surface->h;

  for (int y = 0; y < height; y++) {
    double yCoord = (y - 0.5 * height) / (double)height;

    for (int x = 0; x < width; x++) {
      double xCoord = (x - 0.5 * width) / (double)width;

      double v1 = sin(xCoord * 10.0 + elapsedTime);
      double v2 = sin(10.0 * (xCoord * sin(elapsedTime / 2.0) +
                              yCoord * cos(elapsedTime / 3.0)) +
                      elapsedTime);

      double cx = xCoord + 0.5 * sin(elapsedTime / 5.0);
      double cy = yCoord + 0.5 * cos(elapsedTime / 3.0);
      double v3 = sin(sqrt(cx * cx + cy * cy + 1.0) + elapsedTime);

      double v = v1 + v2 + v3;
      double g = cos(v * M_PI) * 0.5 + 0.5;
      double b = sin(v * M_PI) * 0.5 + 0.5;

      SetPixelSDLSurface(surface, x, y, 255, (Uint8)(g * 255),
                         (Uint8)(b * 255));
    }
  }
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  // TODO: Make these configurable
  const int width = 320;
  const int height = 240;

  // TODO: Measure actual delta time per frame and ensure
  // that the application runs at target frame rate
  const double timeStepSeconds = 0.01666666667;
  double elapsedTime = 0.0;

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

    elapsedTime += timeStepSeconds;

    if (LockSDLSurface(surface) != 0) {
      fprintf(stderr, "SDL_LockSurface error: %s", SDL_GetError());
      break;
    }

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        SetPixelSDLSurface(surface, x, y, 0, 0, 0);
      }
    }

    DrawFrame(surface, elapsedTime);

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

void SetPixelSDLSurface(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g,
                        Uint8 b) {
  int width = surface->w;
  int height = surface->h;

  assert(x >= 0 && x < width && y >= 0 && y < height);

  Uint32 *pixels = (Uint32 *)surface->pixels;
  int index = (y * surface->pitch / surface->format->BytesPerPixel) + x;
  pixels[index] = SDL_MapRGB(surface->format, r, g, b);
}
