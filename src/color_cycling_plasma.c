#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WINDOW_TITLE "Color Cycling Plasma"
#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
#define DEFAULT_REFRESH_RATE 60
#define PALETTE_SIZE 256
#define PI 3.1415926535897932384626433832795

#define LogError(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define LogInfo(...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

SDL_DisplayMode displayMode;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
Uint32 *pixelBuffer = NULL;
Uint32 *plasmaBuffer = NULL;
Uint32 palette[PALETTE_SIZE];

int width = DEFAULT_WIDTH;
int height = DEFAULT_HEIGHT;
int fullscreen = 0;

double Max(double value, double max) {
    return value < max ? value : max;
}

int Get1DArrayIndex(int x, int y, int width) {
    return (y * width) + x;
}

Uint32 RGBToUint32(Uint8 r, Uint8 g, Uint8 b) {
    return (Uint32)((r << 16) + (g << 8) + b);
}

double GetElapsedTimeSecs(Uint64 start, Uint64 end) {
    return (double)(end - start) / SDL_GetPerformanceFrequency();
}

double GetElapsedTimeMs(Uint64 start, Uint64 end) {
    return (double)((end - start) * 1000.0) / SDL_GetPerformanceFrequency();
}

int GetDisplayRefreshRate(SDL_DisplayMode displayMode) {
    int result = displayMode.refresh_rate;

    if (result == 0) {
        return DEFAULT_REFRESH_RATE;
    }

    return result;
}

int InitSDL(void) {
    SDL_Init(SDL_INIT_VIDEO);

    if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0) {
        return -1;
    }

    window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, width, height,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        return -1;
    }
    LogInfo("window created with size %dx%d", width, height);

    if (fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    SDL_ShowCursor(SDL_DISABLE);

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        return -1;
    }
    LogInfo("renderer created with logical size %dx%d", width, height);

    SDL_RenderSetLogicalSize(renderer, width, height);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                                SDL_TEXTUREACCESS_STREAMING, width, height);
    if (texture == NULL) {
        return -1;
    }

    return 0;
}

void InitPalette(void) {
    for (int x = 0; x < PALETTE_SIZE; x++) {
        Uint8 r = (Uint8)Max(128.0 + 128 * sin(PI * x / 32.0), 255);
        Uint8 b = (Uint8)Max(128.0 + 128 * sin(PI * x / 64.0), 255);
        palette[x] = RGBToUint32(r, 0, b);
    }
}

void InitPlasma(void) {
    double halfWidth = width / 2.0;
    double halfHeight = height / 2.0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double color = 128.0 + (128.0 * sin(x / 16.0));
            color += 128.0 + (128.0 * sin(y / 8.0));
            color += 128.0 + (128.0 * sin((x + y) / 16.0));
            color += 128.0 +
                     (128.0 *
                      sin(sqrt((double)((x - halfWidth) * (x - halfWidth) +
                                        (y - halfHeight) * (y - halfHeight))) /
                          8.0));
            // Uncomment for some weird shit
            // color += 128.0 + (128.0 * sin((x * y) / 128.0));
            color += 128.0 + (128.0 * sin(sqrt((double)(x * x + y * y)) / 8.0));

            int index = Get1DArrayIndex(x, y, width);
            plasmaBuffer[index] = (Uint32)color / 8;
        }
    }
}

void DrawFrame(double elapsedTimeInMs) {
    int paletteShift = (int)(elapsedTimeInMs / 32.0);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = Get1DArrayIndex(x, y, width);

            pixelBuffer[index] =
                palette[(plasmaBuffer[index] + paletteShift) % PALETTE_SIZE];
        }
    }
}

void DestroySDL(void) {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    char opt;
    while ((opt = getopt(argc, argv, ":w:h:f")) != -1) {
        switch (opt) {
        case 'w':
            // Obviously not proper use of strtol, but, thats fine
            // for this simple program.
            width = strtol(optarg, (char **)NULL, 10);
            if (width == 0) {
                fprintf(stderr, "invalid value for w: %s\n", optarg);
                return EXIT_FAILURE;
            }
            break;
        case 'h':
            height = strtol(optarg, (char **)NULL, 10);
            if (height == 0) {
                fprintf(stderr, "invalid value for h: %s\n", optarg);
                return EXIT_FAILURE;
            }
            break;
        case 'f':
            fullscreen = 1;
            break;
        }
    }

    if (InitSDL() != 0) {
        fprintf(stderr, "error initializing SDL, %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    int refreshRate = GetDisplayRefreshRate(displayMode);
    const double targetSecsPerFrame = 1.0 / (double)refreshRate;
    LogInfo("display refresh rate %d, target secs per frame %f", refreshRate,
            targetSecsPerFrame);

    pixelBuffer = calloc(width * height, sizeof(*pixelBuffer));
    if (pixelBuffer == NULL) {
        LogError("failed to calloc pixel buffer %dx%d", width, height);
        return EXIT_FAILURE;
    }
    plasmaBuffer = calloc(width * height, sizeof(*plasmaBuffer));
    if (plasmaBuffer == NULL) {
        LogError("failed to calloc plasma buffer %dx%d", width, height);
        return EXIT_FAILURE;
    }

    InitPalette();
    InitPlasma();

    double elapsedTimeMs = 0.0;
    Uint64 lastCounter = SDL_GetPerformanceCounter();
    Uint64 metricsPrintCounter = SDL_GetPerformanceCounter();
    SDL_Event event;
    int isRunning = 1;

    while (isRunning) {
        SDL_PollEvent(&event);
        switch (event.type) {
        case SDL_QUIT:
            isRunning = 0;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                isRunning = 0;
            }
            break;
        }

        elapsedTimeMs += targetSecsPerFrame * 1000.0;

        DrawFrame(elapsedTimeMs);

        // Manually cap the frame rate
        while (GetElapsedTimeSecs(lastCounter, SDL_GetPerformanceCounter()) <
               targetSecsPerFrame) {
        }
        assert(GetElapsedTimeSecs(lastCounter, SDL_GetPerformanceCounter()) >=
               targetSecsPerFrame);

        Uint64 endCounter = SDL_GetPerformanceCounter();

        SDL_UpdateTexture(texture, NULL, pixelBuffer,
                          width * sizeof(*pixelBuffer));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        double msPerFrame = GetElapsedTimeMs(lastCounter, endCounter);
        double fps = (double)SDL_GetPerformanceFrequency() /
                     (double)(endCounter - lastCounter);

        if (GetElapsedTimeMs(metricsPrintCounter, SDL_GetPerformanceCounter()) >
            1000.0) {
            printf("ms/f: %f, fps: %f\r", msPerFrame, fps);
            fflush(stdout);
            metricsPrintCounter = SDL_GetPerformanceCounter();
        }

        lastCounter = endCounter;
    }

    free(plasmaBuffer);
    free(pixelBuffer);
    DestroySDL();

    return EXIT_SUCCESS;
}
