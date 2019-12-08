#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_TITLE "Plasma"
#define DEFAULT_REFRESH_RATE 60

SDL_Window *CreateSDLWindow(int width, int height);
SDL_Renderer *CreateSDLRenderer(SDL_Window *window, int width, int height);
SDL_Surface *CreateSDLSurface(int width, int height);
SDL_Texture *CreateSDLTexture(SDL_Renderer *renderer, int width, int height);

int GetDisplayRefreshRate(SDL_DisplayMode displayMode);
int LockSDLSurface(SDL_Surface *surface);
void SetPixelSDLSurface(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g,
                        Uint8 b);
void ClearPixelsSDLSurface(SDL_Surface *surface);
double GetElapsedTimeSecs(Uint64 start, Uint64 end);
double GetElapsedTimeMs(Uint64 start, Uint64 end);

void DrawFrame(SDL_Surface *surface, double elapsedTime) {
  int width = surface->w;
  int height = surface->h;

  for (int y = 0; y < height; y++) {
    double yNorm = (y - 0.5 * height) / (double)height;

    for (int x = 0; x < width; x++) {
      double xNorm = (x - 0.5 * width) / (double)width;

      double v = 0.0;
      double xNormScaled = xNorm * 10.0;
      double yNormScaled = yNorm * 10.0;

      v += sin(xNormScaled + elapsedTime);
      v += sin((yNormScaled + elapsedTime) * 0.5);
      v += sin((xNormScaled + yNormScaled + elapsedTime) * 0.5);

      double cx = xNorm + 0.5 * sin(elapsedTime * 0.2);
      double cy = yNorm + 0.5 * cos(elapsedTime * 0.3);

      v += sin(sqrt(100.0 * (cx * cx + cy * cy) + 1.0) + elapsedTime);
      v *= 0.5;

      double r = sin(v * M_PI) * 0.5 + 0.5;
      double g = sin(v * M_PI + 2.0 * M_PI / 3.0) * 0.5 + 0.5;
      double b = sin(v * M_PI + 4.0 * M_PI / 3.0) * 0.5 + 0.5;

      SetPixelSDLSurface(surface, x, y, (Uint8)(r * 255), (Uint8)(g * 255),
                         (Uint8)(b * 255));
    }
  }
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  // TODO: Make these configurable
  const int width = 320;
  const int height = 240;

  SDL_DisplayMode displayMode;
  if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "SDL_GetDesktopDisplayMode error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }
  int refreshRate = GetDisplayRefreshRate(displayMode);
  const double targetSecsPerFrame = 1.0 / (double)refreshRate;
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
              "display refresh rate %d, target secs per frame %f", refreshRate,
              targetSecsPerFrame);

  SDL_Window *window = CreateSDLWindow(width, height);
  if (window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateSDLWindow error: %s",
                 SDL_GetError());
    return EXIT_FAILURE;
  }
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "window created with size %dx%d",
              width, height);

  SDL_Renderer *renderer = CreateSDLRenderer(window, width, height);
  if (renderer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateSDLRenderer error: %s",
                 SDL_GetError());
    return EXIT_FAILURE;
  }
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
              "renderer created with logical size %dx%d", width, height);

  SDL_Surface *surface = CreateSDLSurface(width, height);
  if (surface == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateSDLSurface error: %s",
                 SDL_GetError());
    return EXIT_FAILURE;
  }
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "surface created with size %dx%d",
              width, height);

  SDL_Texture *texture = CreateSDLTexture(renderer, width, height);
  if (texture == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateSDLTexture error: %s",
                 SDL_GetError());
    return EXIT_FAILURE;
  }

  double elapsedTime = 0.0;
  Uint64 lastCounter = SDL_GetPerformanceCounter();
  Uint64 metricsPrintCounter = SDL_GetPerformanceCounter();
  SDL_Event event;

  while (1) {
    SDL_PollEvent(&event);

    if (event.type == SDL_QUIT) {
      break;
    }

    elapsedTime += targetSecsPerFrame;

    Uint64 drawStartCounter = SDL_GetPerformanceCounter();
    if (LockSDLSurface(surface) != 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LockSDLSurface error: %s",
                   SDL_GetError());
      break;
    }

    ClearPixelsSDLSurface(surface);
    DrawFrame(surface, elapsedTime);

    SDL_UnlockSurface(surface);
    Uint64 drawEndCounter = SDL_GetPerformanceCounter();

    // Manually cap the frame rate
    while (GetElapsedTimeSecs(lastCounter, SDL_GetPerformanceCounter()) <
           targetSecsPerFrame) {
    }
    assert(GetElapsedTimeSecs(lastCounter, SDL_GetPerformanceCounter()) >=
           targetSecsPerFrame);

    Uint64 endCounter = SDL_GetPerformanceCounter();

    SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    double msPerFrame = GetElapsedTimeMs(lastCounter, endCounter);
    double fps = (double)SDL_GetPerformanceFrequency() /
                 (double)(endCounter - lastCounter);
    double msPerDraw = GetElapsedTimeMs(drawStartCounter, drawEndCounter);

    if (GetElapsedTimeMs(metricsPrintCounter, SDL_GetPerformanceCounter()) >
        1000.0) {
      printf("ms/f: %f, fps: %f, draw-ms/f: %f\r", msPerFrame, fps, msPerDraw);
      fflush(stdout);
      metricsPrintCounter = SDL_GetPerformanceCounter();
    }

    lastCounter = endCounter;
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

int GetDisplayRefreshRate(SDL_DisplayMode displayMode) {
  int result = displayMode.refresh_rate;

  if (result == 0) {
    return DEFAULT_REFRESH_RATE;
  }

  return result;
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

  int pixelInBounds = x >= 0 && x < width && y >= 0 && y < height;
  assert(pixelInBounds);

  if (pixelInBounds) {
    Uint32 *pixels = (Uint32 *)surface->pixels;
    int index = (y * surface->pitch / surface->format->BytesPerPixel) + x;
    pixels[index] = SDL_MapRGBA(surface->format, r, g, b, 255);
  }
}

void ClearPixelsSDLSurface(SDL_Surface *surface) {
  int size = surface->w * surface->h;
  Uint32 *pixels = (Uint32 *)surface->pixels;

  for (int i = 0; i < size; i++) {
    pixels[i] = 0;
  }
}

double GetElapsedTimeSecs(Uint64 start, Uint64 end) {
  return (double)(end - start) / SDL_GetPerformanceFrequency();
}

double GetElapsedTimeMs(Uint64 start, Uint64 end) {
  return (double)((end - start) * 1000.0) / SDL_GetPerformanceFrequency();
}
