#include <mschne.h>
#include <math.h>

int main() {
        static app_window win;
        win.width = 1920;
        win.height = 1080;
        win.winName = "GLB Render";
        vec3 rotation1 = {90.0f, -180.0f, -270.0f};
        vec3 rotation2 = {0.0f, 90.0f, 0.0f};
        m_createWin(win);
        Model test_model = m_loadModel("assets/models/anvil_test.glb", NULL);
        while (m_pollEvents()) {
                vec3 pos1 = {-1.0f, -1.0f, -5.0f};
                m_startFrame();
                m_drawModel(test_model, pos1, rotation1);
                m_endFrame();
        }
        m_freeModel(test_model);
        m_destroyWin();
        return 0;
}
