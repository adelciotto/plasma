#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WINDOW_TITLE "RGB Plasma"
#define DEFAULT_WIDTH 128
#define DEFAULT_HEIGHT 128
#define DEFAULT_SCALE 4
#define DEFAULT_REFRESH_RATE 60
#define PLASMA_SCALE 20.0
#define PLASMA_SCALE_HALF PLASMA_SCALE * 0.5

#define LogError(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define LogInfo(...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

SDL_DisplayMode displayMode;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
Uint32 *pixelBuffer = NULL;

int width = DEFAULT_WIDTH;
int height = DEFAULT_HEIGHT;
int scale = DEFAULT_SCALE;
int fullscreen = 0;
int interactive = 0;

double mouseX = -0.5;
double mouseY = -0.5;

double Min(double value, double min) {
    return value < min ? value : min;
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

    window = SDL_CreateWindow(
        WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width * scale, height * scale, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        return -1;
    }
    LogInfo("window created with size %dx%d", width, height);

    if (fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    if (!interactive) {
        SDL_ShowCursor(SDL_DISABLE);
    }

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

void DrawFrame(double elapsedTimeInSecs) {
    double t = elapsedTimeInSecs;

    for (int yi = 0; yi < height; yi++) {
        double y = (0.5 + yi / (double)height - 1.0) * PLASMA_SCALE -
                   PLASMA_SCALE_HALF;

        for (int xi = 0; xi < width; xi++) {
            double x = (0.5 + xi / (double)width - 1.0) * PLASMA_SCALE -
                       PLASMA_SCALE_HALF;

            double val = sin(y + t);
            val += sin((x + t) * 0.5);
            val += sin((x + y + t) * 0.5);
            double cx = x + PLASMA_SCALE_HALF * (sin(t * 0.33));
            double cy = y + PLASMA_SCALE_HALF * (cos(t * 0.5));
            val += sin(sqrt(cx * cx + cy * cy + 1.0) + t);
            val *= 0.5;

            double r, g, b;
            if (interactive) {
                double dist = sqrt((x - mouseX) * (x - mouseX) +
                                   (y - mouseY) * (y - mouseY));
                r = sin((val + sin(dist * 2 + t)) * M_PI) * 0.5 + 0.5;
                g = sin(val * M_PI + 2.0 * M_PI * 0.33) * 0.5 + 0.5;
                b = sin((val + cos(dist + t * 0.33)) * M_PI +
                        4.0 * M_PI * 0.33) *
                        0.5 +
                    0.5;
            } else {
                r = sin(val * M_PI) * 0.5 + 0.5;
                g = sin(val * M_PI + 2.0 * M_PI * 0.33) * 0.5 + 0.5;
                b = sin(val * M_PI + 4.0 * M_PI * 0.33) * 0.5 + 0.5;
            }

            Uint8 ri = (Uint8)Min(r * 255, 255);
            Uint8 gi = (Uint8)Min(g * 255, 255);
            Uint8 bi = (Uint8)Min(b * 255, 255);

            pixelBuffer[Get1DArrayIndex(xi, yi, width)] =
                RGBToUint32(ri, gi, bi);
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
    while ((opt = getopt(argc, argv, ":w:h:s:fi")) != -1) {
        switch (opt) {
        case 'w':
            // Obviously not proper use of strtol, but, thats fine
            // for this simple program.
            width = strtol(optarg, (char **)NULL, 10);
            if (width == 0) {
                fprintf(stderr, "invalid value for width: %s\n", optarg);
                return EXIT_FAILURE;
            }
            break;
        case 'h':
            height = strtol(optarg, (char **)NULL, 10);
            if (height == 0) {
                fprintf(stderr, "invalid value for height: %s\n", optarg);
                return EXIT_FAILURE;
            }
        case 's':
            scale = strtol(optarg, (char **)NULL, 10);
            if (scale == 0) {
                fprintf(stderr, "invalid value for scale: %s\n", optarg);
                return EXIT_FAILURE;
            }
            break;
        case 'f':
            fullscreen = 1;
            break;
        case 'i':
            interactive = 1;
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
        case SDL_MOUSEMOTION: {
            int x, y;
            SDL_GetMouseState(&x, &y);
            mouseX = 0.5 + x / (double)width - 1.0;
            mouseY = 0.5 + y / (double)height - 1.0;
            break;
        }
        }

        elapsedTimeMs += targetSecsPerFrame;

        Uint64 drawStartCounter = SDL_GetPerformanceCounter();
        DrawFrame(elapsedTimeMs);
        Uint64 drawEndCounter = SDL_GetPerformanceCounter();

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
        double msPerDraw = GetElapsedTimeMs(drawStartCounter, drawEndCounter);

        if (GetElapsedTimeMs(metricsPrintCounter, SDL_GetPerformanceCounter()) >
            1000.0) {
            printf("ms/f: %f, fps: %f, draw-ms/f: %f\r", msPerFrame, fps,
                   msPerDraw);
            fflush(stdout);
            metricsPrintCounter = SDL_GetPerformanceCounter();
        }

        lastCounter = endCounter;
    }

    free(pixelBuffer);
    DestroySDL();

    return EXIT_SUCCESS;
}
