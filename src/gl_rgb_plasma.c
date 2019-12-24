#include <GL/glew.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WINDOW_TITLE "RGB Plasma GL"
#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
#define DEFAULT_REFRESH_RATE 60
#define VERTEX_SHADER_PATH "src/shaders/gl_rgb_plasma.vert"
#define FRAGMENT_SHADER_PATH "src/shaders/gl_rgb_plasma.frag"

#define LogError(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define LogInfo(...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

SDL_DisplayMode gDisplayMode;
SDL_Window *gWindow = NULL;

SDL_GLContext *gContext = NULL;
GLuint gProgramId = 0;
GLuint gVAO = 0;
GLuint gVBO = 0;
GLuint gEBO = 0;
GLint gUniformTimeLocation = -1;
GLint gUniformResolutionLocation = -1;
GLint gUniformScaleLocation = -1;

int gWidth = DEFAULT_WIDTH;
int gHeight = DEFAULT_HEIGHT;
int gFullscreen = 0;

double GetElapsedTimeSecs(Uint64 start, Uint64 end) {
    return (double)(end - start) / SDL_GetPerformanceFrequency();
}

double GetElapsedTimeMs(Uint64 start, Uint64 end) {
    return (double)((end - start) * 1000.0) / SDL_GetPerformanceFrequency();
}

int GetDisplayRefreshRate(SDL_DisplayMode gDisplayMode) {
    int result = gDisplayMode.refresh_rate;

    if (result == 0) {
        return DEFAULT_REFRESH_RATE;
    }

    return result;
}

int InitSDL(void) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    if (SDL_GetDesktopDisplayMode(0, &gDisplayMode) != 0) {
        return -1;
    }

    gWindow = SDL_CreateWindow(
        WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, gWidth,
        gHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (gWindow == NULL) {
        return -1;
    }
    LogInfo("window created with size %dx%d", gWidth, gHeight);

    gContext = SDL_GL_CreateContext(gWindow);
    if (gContext == NULL) {
        return -1;
    }

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        LogError("error initializing GLEW! %s", glewGetErrorString(glewError));
    }

    if (SDL_GL_SetSwapInterval(1) < 0) {
        LogInfo("warning: unable to set vsync. %s", SDL_GetError());
    }

    if (gFullscreen) {
        SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    SDL_ShowCursor(SDL_DISABLE);

    return 0;
}

void LogShaderError(GLuint shader) {
    if (glIsShader(shader)) {
        int infoLogLength = 0;
        int maxLength = infoLogLength;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        char *infoLog = malloc(maxLength * sizeof(*infoLog));
        if (infoLog == NULL) {
            LogError("failed to malloc error log for gl shader %d of size %d",
                     shader, maxLength);
        }

        glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
        if (infoLogLength > 0) {
            LogError("failed to compile shader %d:\n%s", shader, infoLog);
        }

        free(infoLog);
    } else {
        LogError("name shader %d is not a shader", shader);
    }
}

void LogProgramError(GLuint program) {
    if (glIsProgram(program)) {
        int infoLogLength = 0;
        int maxLength = infoLogLength;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        char *infoLog = malloc(maxLength * sizeof(*infoLog));
        if (infoLog == NULL) {
            LogError("failed to malloc error log for gl program %d of size %d",
                     program, maxLength);
        }

        glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
        if (infoLogLength > 0) {
            LogError("failed to link program %d:\n%s", program, infoLog);
        }

        free(infoLog);
    } else {
        LogError("name program %d is not a program", program);
    }
}

char *ReadFile(const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *buffer = malloc((fileSize + 1) * sizeof(*buffer));
    if (buffer == NULL) {
        return NULL;
    }

    size_t result = fread(buffer, 1, fileSize, file);
    if (result != fileSize) {
        return NULL;
    }

    fclose(file);

    buffer[fileSize] = 0;
    return buffer;
}

GLboolean compileShader(GLenum type, const GLchar *source[],
                        GLuint *outShader) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, source, NULL);
    glCompileShader(shader);

    *outShader = shader;

    GLint shaderCompiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);
    if (shaderCompiled != GL_TRUE) {
        return GL_FALSE;
    }

    return GL_TRUE;
}

int InitGL(void) {
    gProgramId = glCreateProgram();

    GLuint vertexShader;
    const char *vertexShaderSource = ReadFile(VERTEX_SHADER_PATH);
    if (vertexShaderSource == NULL) {
        LogError("could not read file %s", VERTEX_SHADER_PATH);
    }
    if (compileShader(GL_VERTEX_SHADER, &vertexShaderSource, &vertexShader) !=
        GL_TRUE) {
        LogShaderError(vertexShader);
        return -1;
    }

    GLuint fragmentShader;
    const char *fragmentShaderSource = ReadFile(FRAGMENT_SHADER_PATH);
    if (fragmentShaderSource == NULL) {
        LogError("could not read file %s", FRAGMENT_SHADER_PATH);
    }
    if (compileShader(GL_FRAGMENT_SHADER, &fragmentShaderSource,
                      &fragmentShader) != GL_TRUE) {
        LogShaderError(fragmentShader);
        return -1;
    }

    glAttachShader(gProgramId, vertexShader);
    glAttachShader(gProgramId, fragmentShader);

    glLinkProgram(gProgramId);
    GLint programSuccess = GL_TRUE;
    glGetProgramiv(gProgramId, GL_LINK_STATUS, &programSuccess);
    if (programSuccess != GL_TRUE) {
        LogProgramError(gProgramId);
        return -1;
    }

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    gUniformTimeLocation = glGetUniformLocation(gProgramId, "uTime");
    if (gUniformTimeLocation == -1) {
        LogError("could not get uniform location for uTime");
        return -1;
    }
    gUniformScaleLocation = glGetUniformLocation(gProgramId, "uScale");
    if (gUniformScaleLocation == -1) {
        LogError("could not get uniform location for uScale");
        return -1;
    }
    gUniformResolutionLocation =
        glGetUniformLocation(gProgramId, "uResolution");
    if (gUniformResolutionLocation == -1) {
        LogError("could not get uniform location for uResolution");
        return -1;
    }

    glViewport(0, 0, gWidth, gHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    GLfloat vertexData[] = {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f};
    GLuint elements[] = {0, 1, 2, 2, 3, 0};

    glGenVertexArrays(1, &gVAO);
    glBindVertexArray(gVAO);

    glGenBuffers(1, &gVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData,
                 GL_STATIC_DRAW);

    glGenBuffers(1, &gEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (void *)0);

    return 0;
}

void DrawFrame(double elapsedTimeSecs) {
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(gProgramId);
    glUniform1f(gUniformScaleLocation, 20.0f);
    glUniform2i(gUniformResolutionLocation, gWidth, gHeight);
    glUniform1f(gUniformTimeLocation, elapsedTimeSecs);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void DestroyGL(void) {
    glDeleteVertexArrays(1, &gVAO);
    glDeleteBuffers(1, &gVBO);
    glDeleteProgram(gProgramId);
}

void DestroySDL(void) {
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    char opt;
    while ((opt = getopt(argc, argv, ":w:h:f")) != -1) {
        switch (opt) {
        case 'w':
            // Obviously not proper use of strtol, but, thats fine
            // for this simple program.
            gWidth = strtol(optarg, (char **)NULL, 10);
            if (gWidth == 0) {
                fprintf(stderr, "invalid value for width: %s\n", optarg);
                return EXIT_FAILURE;
            }
            break;
        case 'h':
            gHeight = strtol(optarg, (char **)NULL, 10);
            if (gHeight == 0) {
                fprintf(stderr, "invalid value for height: %s\n", optarg);
                return EXIT_FAILURE;
            }
            break;
        case 'f':
            gFullscreen = 1;
            break;
        }
    }

    if (InitSDL() != 0) {
        fprintf(stderr, "error initializing SDL, %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (InitGL() != 0) {
        return EXIT_FAILURE;
    }

    int refreshRate = GetDisplayRefreshRate(gDisplayMode);
    const double targetSecsPerFrame = 1.0 / (double)refreshRate;
    LogInfo("display refresh rate %d, target secs per frame %f", refreshRate,
            targetSecsPerFrame);

    double elapsedTimeSecs = 0.0;
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
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                gWidth = event.window.data1;
                gHeight = event.window.data2;
                glViewport(0, 0, gWidth, gHeight);
            }
            break;
        }

        elapsedTimeSecs += targetSecsPerFrame;

        Uint64 drawStartCounter = SDL_GetPerformanceCounter();
        DrawFrame(elapsedTimeSecs);
        Uint64 drawEndCounter = SDL_GetPerformanceCounter();

        // Manually cap the frame rate
        while (GetElapsedTimeSecs(lastCounter, SDL_GetPerformanceCounter()) <
               targetSecsPerFrame) {
        }
        assert(GetElapsedTimeSecs(lastCounter, SDL_GetPerformanceCounter()) >=
               targetSecsPerFrame);

        Uint64 endCounter = SDL_GetPerformanceCounter();

        SDL_GL_SwapWindow(gWindow);

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

    DestroyGL();
    DestroySDL();

    return EXIT_SUCCESS;
}
