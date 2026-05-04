#pragma once
#include <stdio.h>
#include <SDL3/SDL.h>
// Global defines and variables for MSCHNE

#if defined(_WIN32)
        #define MAPI __declspec(dllexport)
#else
        #define MAPI __attribute__((visibility("default")))
#endif

// SDL
#define SDL_FLAGS SDL_INIT_VIDEO

// Data types

typedef int b8;
#define TRUE            1
#define FALSE           0

// Window defs

#define DEF_WIDTH       1280
#define DEF_HEIGHT      720
#define DEF_WIN_NAME    "MSCHNE Window"

// Window structs

typedef struct app_window {
        int width;
        int height;
        const char* winName;
} app_window;

// Engine structs

typedef struct game_inst {
        SDL_Window *window;
        SDL_Renderer *renderer;
} game_inst;