#include "../logger.h"
#include "../../globals.h"
#include "platform_linux.h"

b8 m_createWin(app_window win) {
        renderer render = {0};
        Plat_InitWindow(win, &render);
        Plat_RenderClear(&render);
        Plat_FreeSDL(&render);
        return TRUE;
}