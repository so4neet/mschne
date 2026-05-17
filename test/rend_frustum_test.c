#include <mschne.h>
#include <math.h>

int main() {
        static app_window win;
        win.width = 1920;
        win.height = 1080;
        win.winName = "FBX Render";

        vec3 rotation2 = {0.0f, 0.0f, 0.0f};
        m_createWin(win);
        Model blackBear = m_loadModel("assets/models/BlackBear.fbx", "assets/textures/BlackBear_Textf.png");
        while (m_pollEvents()) {
                vec3 pos2 = {3.0f, 0.0f, -5.0f};
                m_startFrame();
                for (int i=0; i<100; i++){
                        m_drawModel(blackBear, (vec3){i+i, pos2[1], pos2[2]}, rotation2);                        
                }
                m_endFrame();
        }
        m_freeModel(blackBear);
        m_destroyWin();
        return 0;
}
