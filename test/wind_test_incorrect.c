#include <mschne.h>

int main() {
        // Empty struct, doesn't contain any window information
        static app_window win;

        // Create the window, because the struct is empty it will use default values
        m_createWin(win);

        // Game loop
        while (m_pollEvents()) {
                m_startFrame();
                m_endFrame();
        }

        // Close window
        m_destroyWin();
        return 0;
}