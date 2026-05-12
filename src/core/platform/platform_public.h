#pragma once

#include "../../globals.h"

b8 m_createWin(app_window win);
void m_draw();
void m_destroyWin();
b8 m_pollEvents();
MAPI void m_startFrame();
MAPI void m_endFrame();
MAPI void m_drawTestCube(vec3 pos);
MAPI void m_updateCam();
MAPI Model m_loadModel(const char* path, const char* tex_path);
MAPI void m_drawModel(Model model, vec3 position, vec3 rotation);
MAPI void m_freeModel(Model model);