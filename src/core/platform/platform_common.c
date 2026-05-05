#include "../logger.h"
#include "../../globals.h"
#include "platform_linux.h"

renderer render = {0};

MAPI b8 m_createWin(app_window win) {
        Plat_InitWindow(win, &render);
        return TRUE;
}

MAPI b8 m_pollEvents(){
        return Plat_Event(&render);
}

MAPI void m_draw() {
        Plat_Draw(&render);
}

MAPI void m_destroyWin() {
        Plat_FreeSDL(&render);
}