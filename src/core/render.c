#include <raylib.h>
#include <stdlib.h>
#include "render.h"
#include "logger.h"
#include "../globals.h"

bool CreateWindow(app_window win) {
        if (win.width == 0) {
                mWarn("Window width not specified, defaulting.");
                win.width = DEF_WIDTH;
        }
        if (win.height == 0) {
                mWarn("Window height not specified, defaulting.");
                win.height = DEF_HEIGHT;
        } 
        if (win.winName == NULL) {
                mWarn("Window name not specified, defaulting.");
                win.winName = DEF_WIN_NAME;
        }
        SetTraceLogLevel(LOG_FATAL);
        InitWindow(win.width, win.height, win.winName);
        if(!IsWindowReady()) {
                mFatal("Couldn't create window!");
                return false;
        } else {
                mInfo("Created window!");
                return true;
        }
}

// This is temporary. The app will determine what's rendering, not the API.
void RenderWindow() {
        while (!WindowShouldClose()) {
                BeginDrawing();
                ClearBackground(BLACK);
                EndDrawing();
        }
        mInfo("Closing Window.");
        CloseWindow();
}