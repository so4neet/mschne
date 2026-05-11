#include <mschne.h>
#include <math.h>

int main() {
        static app_window win;
        win.width = 1920;
        win.height = 1080;
        win.winName = "Cube Render";

        float time = 0.0f;
        m_createWin(win);

        while (m_pollEvents()) {
                time += 0.01f;

                vec3 pos = {
                        sinf(time) * 2.0f,
                        sinf(time) * 1.0f,
                        -3.0f
                };
                m_startFrame();
                m_drawTestCube(pos);
                m_endFrame();
        }

        m_destroyWin();
        return 0;
}
