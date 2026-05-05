#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include "../logger.h"
#include "../../globals.h"
#include "platform_linux.h"

// Most of this code was created by following "Open Window - Beginners Guide to SDL3 in C - Part 1 | Programming Rainbow"

void Plat_FreeSDL(renderer *render) {
        if(render->window) {
                SDL_DestroyWindow(render->window);
                render->window = NULL;
        }

        if(render->renderer) {
                SDL_DestroyRenderer(render->renderer);
                render->renderer = NULL;
        }
        SDL_Quit();
}

b8 Plat_Event(renderer *render) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
                switch (event.type) {
                        case SDL_EVENT_QUIT:
                                render->isRunning = false;
                                break;
                        default:
                                break;
                }
        }
        return render->isRunning;
}

void Plat_Draw(renderer *render) {
        SDL_RenderClear(render->renderer);
        SDL_RenderPresent(render->renderer);
        SDL_Delay(16);
}

b8 Plat_InitWindow(app_window win, renderer *render) {
        // Initialize SDL
        if (!SDL_Init(SDL_FLAGS)) {
                mFatal("Couldn't init renderer, aborting.");
                return FALSE;
        }
        // Check if w,h,n are defined
        if (win.width == 0) {
                mWarn("Window width not defined, defaulting...");
                win.width = DEF_WIDTH;
        }
        if (win.height == 0) {
                mWarn("Window height not defined, defaulting...");
                win.height = DEF_HEIGHT;
        }
        if (win.winName == NULL) {
                mWarn("Window name not defined, defaulting...");
                win.winName = DEF_WIN_NAME;
        }

        // Create the window
        render->window = SDL_CreateWindow(win.winName, win.width, win.height, 0);
        if (!render->window) {
                mFatal("Couldn't create window, aborting.");
                return FALSE;
        } else {
                mInfo("Initialized window.");
        }

        render->renderer = SDL_CreateRenderer(render->window, NULL);
        if (!render->renderer) {
                mFatal("Couldn't create renderer, aborting.");
                return FALSE;
        } else {
                render->isRunning = true;
                mInfo("Initialized renderer.");
        }
        mInfo("Opened window.");
        Plat_Draw(render);
        return TRUE;
}

