#include <mschne.h>
#include <math.h>

int main() {
        static app_window win;
        win.width = 1920;
        win.height = 1080;
        win.winName = "FBX Render";

        m_createWin(win);
        Model test_model = m_loadModel("assets/models/house.fbx", "assets/textures/cottage_diffuse.png");
        Model blackBear = m_loadModel("assets/models/BlackBear.fbx", "assets/textures/BlackBear_Textf.png");
        while (m_pollEvents()) {

                vec3 pos1 = {-1.0f, 0.0f, -5.0f};
                vec3 pos2 = {3.0f, 0.0f, -5.0f};
                m_startFrame();
                m_drawModel(test_model, pos1);
                m_drawModel(blackBear, pos2);
                m_endFrame();
        }
        m_freeModel(test_model);
        m_freeModel(blackBear);
        m_destroyWin();
        return 0;
}
