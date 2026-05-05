# mschne
A game engine written in C.

MSCHNE uses SDL for its rendering, and compiles as a dynamic library. 


## Hello MSCHNE
`#include <mschne.h>

int main() {
        // Fills struct with window information
        static app_window win;
        win.width = 1920;
        win.height = 1080;
        win.winName = "Hello MSCHNE";

        // Create the window
        m_createWin(win);

        // Game loop
        while (m_pollEvents()) {
                m_draw();  // Draws frame to the screen
        }

        // Close window
        mInfo("Destroyed window.");
        m_destroyWin();
        return 0;
}`
