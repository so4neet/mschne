#pragma once 

#include "../globals.h"
#include <stdbool.h>

typedef struct app_window {
        int width;
        int height;
        const char* winName;
} app_window;

bool CreateWindow(app_window win);
void RenderWindow();