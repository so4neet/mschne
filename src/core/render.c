#include <raylib.h>
#include <stdlib.h>
#include "logger.h"
#include "../globals.h"


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