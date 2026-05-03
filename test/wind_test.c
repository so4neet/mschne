#include <core/logger.h>
#include <core/render.h>

int main() {
        mInfo("Trying to open window...");
        static app_window win;
        CreateWindow(win);
        RenderWindow();
}