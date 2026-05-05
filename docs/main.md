# MSCHNE Documentation

MSCHNE is a game engine as a library for C. This documentation will cover all of the features of the engine, and how to use them. No generative AI was used for MSCHNE in either the code, or the documentation.

> [!NOTE]
> This documentation, as well as the engine, is not complete. Major features *will* be missing, but will be added and updated as development continues.

## Table of Contents

- [How MSCHNE Works](#how-mschne-works)
- [Project Setup](#project-setup)
  - [Logging](#logging)
  - [Creating a window](#creating-a-window)

## How MSCHNE Works

Although MSCHNE *does* have a map editor, the actual *game* being created is written in C using libmschne. The engine contains functions to handle basic logic, rendering, model loading, and everything you would expect from an engine, allowing the developer to have better flexability than all-in-one engines. If you are looking for a drag-n-drop, visual scripting experience, then this is likely not the engine for you. (This section will be updated once more is added to the engine, giving an overview of the main features).

## Project Setup

Setting up MSCHNE for development is a fairly simple ordeal.
> [!NOTE]
> This will change to tagged releases once the engine is usable, the tagged releases will contain only the engine.

 - Clone the repo 
 
   - `git clone https://github.com/so4neet/mschne`

 - Create a folder for your project
 - Setup your build system (make, cMake, etc. For this example, i'm just using clang).

   - `clang [source-files] -g -fdeclspec -fPIC -Isrc -I../src/ -L[build-directory] -lmschne -Wl,-rpath,.`

## Logging

MSCHNE has built-in logging for 5 types: Info, Debug, Warnings, Errors, and Fatal. These can be called by using their respective functions: 
```c
mInfo("This is info");
mDebug("This is a debug message");
mWarn("This is a warning message");
mErr("This is an error message");
mFatal("This is a fatal crash message");
```

These can be supressed at compile-time by passing through the build flag `-D=MSUPPRESS`.

## Creating A Window

MSCHNE uses SDL3 under the hood, wrapped in helper functions to abstract the windowing code.

To create a window, you first need to pass through the `app_window` struct, which has the three elements `width`, `height`, and `winName`. If left blank, the engine will default to 1280x720, with the title "MSCHNE Window". A basic example of this would be:
```c
static app_window win;
win.width = 1280;
win.height = 720;
win.winName = "MSCHNE Window";
```

To actually initialize the window, use the function `m_createWin()` and pass through your app_window struct. Then, to start your game loop, use `m_pollEvents()` in a while loop. This will keep the window open and ready to draw unless the user requests close, or the game destroys the window. To draw update the window and draw the current frame, use `m_draw()`. a basic example of this would be:
```c
while (m_pollEvents()) {
        // Game loop here.
        m_draw();  // Draw at the end of the loop.
}
```

When the window is closed, the memory allocated needs to be freed, and SDL needs to shutdown. To do this, outside of your game loop use `m_destroyWin()`. A full example of a "Hello World" for blank MSCHNE window would look like this:
```c
#include <mschne.h>

int main() {
        static app_window win;
        win.width = 800;
        win.height = 600;
        win.winName = "Hello World";

        m_createWin(win);

        while (m_pollEvents()) {
                m_draw();
        }

        m_destroyWin();
        return 0;
}
```