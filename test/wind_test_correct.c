#include <mschne.h>

int main() {
        // Fills struct with window information
        static app_window win;
        win.width = 1920;
        win.height = 1080;
        win.winName = "MSCHNE Correct";

        // Create the window
        m_createWin(win);

        // Game loop
        while (m_pollEvents()) {
                m_draw();
        }

        // Close window
        mInfo("Destroyed window.");
        m_destroyWin();
        return 0;
}