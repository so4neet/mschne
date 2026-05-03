#include <core/logger.h>
#include <core/render.h>

int main() {
        mInfo("Trying to open window...");
        static app_window win;
        win.width = 1920;
        win.height = 1080;
        win.winName = "MSCHNE Correct";
        CreateWindow(win);
        RenderWindow();
}