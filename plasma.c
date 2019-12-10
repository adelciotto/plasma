#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_TITLE "Plasma"
#define DEFAULT_REFRESH_RATE 60
#define PALETTE_SIZE 256

SDL_Window *CreateSDLWindow(int width, int height);
SDL_Renderer *CreateSDLRenderer(SDL_Window *window, int width, int height);
SDL_Texture *CreateSDLTexture(SDL_Renderer *renderer, int width, int height);
int GetDisplayRefreshRate(SDL_DisplayMode displayMode);
double GetElapsedTimeSecs(Uint64 start, Uint64 end);
double GetElapsedTimeMs(Uint64 start, Uint64 end);

int Get1DCoordFrom2D(int x, int y, int width) { return (y * width) + x; }
double Minf(double value, double min) { return value < min ? value : min; }
Uint32 RGBToUint32(Uint8 r, Uint8 g, Uint8 b);

void DrawFrame(Uint32 *pixelBuffer, int width, int height, Uint32 *plasma,
               Uint32 palette[PALETTE_SIZE], double elapsedTime) {
  int paletteShift = (int)(elapsedTime / 10.0);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int index = Get1DCoordFrom2D(x, y, width);
      pixelBuffer[index] =
          palette[(plasma[index] + paletteShift) % PALETTE_SIZE];
    }
  }
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  // TODO: Make these configurable
  const int width = 256;
  const int height = 256;

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

  SDL_Window *window = CreateSDLWindow(width * 3, height * 3);
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

  SDL_Texture *texture = CreateSDLTexture(renderer, width, height);
  if (texture == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateSDLTexture error: %s",
                 SDL_GetError());
    return EXIT_FAILURE;
  }

  Uint32 *pixelBuffer = calloc(width * height, sizeof(*pixelBuffer));
  if (pixelBuffer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "failed to calloc pixel buffer %dx%d", width, height);
    return EXIT_FAILURE;
  }
  Uint32 *plasma = calloc(width * height, sizeof(*plasma));
  if (plasma == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "failed to calloc plasma buffer %dx%d", width, height);
    return EXIT_FAILURE;
  }

  double halfWidth = width / 2.0;
  double halfHeight = height / 2.0;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      double color = 128.0 + (128.0 * sin(x / 16.0));
      color += 128.0 + (128.0 * sin(y / 32.0));
      color +=
          128.0 +
          (128.0 * sin(sqrt((double)((x - halfWidth) * (x - halfWidth) +
                                     (y - halfHeight) * (y - halfHeight))) /
                       8.0));
      color += 128.0 + (128.0 * sin(sqrt((double)(x * x + y * y)) / 8.0));

      int index = Get1DCoordFrom2D(x, y, width);
      plasma[index] = (Uint32)color / 4;
    }
  }

  Uint32 palette[PALETTE_SIZE];
  for (int x = 0; x < PALETTE_SIZE; x++) {
    Uint8 r = (int)Minf(128.0 + 128 * sin(M_PI * x / 16.0), 255);
    Uint8 g = (int)Minf(128.0 + 128 * sin(M_PI * x / 128.0), 255);
    palette[x] = RGBToUint32(r, g, 0);
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

    elapsedTime += targetSecsPerFrame * 1000.0;

    Uint64 drawStartCounter = SDL_GetPerformanceCounter();

    DrawFrame(pixelBuffer, width, height, plasma, palette, elapsedTime);

    Uint64 drawEndCounter = SDL_GetPerformanceCounter();

    // Manually cap the frame rate
    while (GetElapsedTimeSecs(lastCounter, SDL_GetPerformanceCounter()) <
           targetSecsPerFrame) {
    }
    assert(GetElapsedTimeSecs(lastCounter, SDL_GetPerformanceCounter()) >=
           targetSecsPerFrame);

    Uint64 endCounter = SDL_GetPerformanceCounter();

    SDL_UpdateTexture(texture, NULL, pixelBuffer, width * sizeof(*pixelBuffer));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);

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

  free(plasma);
  free(pixelBuffer);
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

SDL_Texture *CreateSDLTexture(SDL_Renderer *renderer, int width, int height) {
  return SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                           SDL_TEXTUREACCESS_STREAMING, width, height);
}

int GetDisplayRefreshRate(SDL_DisplayMode displayMode) {
  int result = displayMode.refresh_rate;

  if (result == 0) {
    return DEFAULT_REFRESH_RATE;
  }

  return result;
}

double GetElapsedTimeSecs(Uint64 start, Uint64 end) {
  return (double)(end - start) / SDL_GetPerformanceFrequency();
}

double GetElapsedTimeMs(Uint64 start, Uint64 end) {
  return (double)((end - start) * 1000.0) / SDL_GetPerformanceFrequency();
}

Uint32 RGBToUint32(Uint8 r, Uint8 g, Uint8 b) {
  // TODO: Provide mask for little endian vs big endian
  return (Uint32)((r << 16) + (g << 8) + b);
}
