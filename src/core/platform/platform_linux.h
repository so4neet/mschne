#pragma once
#include <stdlib.h>
#include "../../globals.h"

b8 Plat_InitWindow(app_window win, renderer *render);
void Plat_FreeSDL(renderer *render);
b8 Plat_Event(renderer *render);
void Plat_Draw(renderer *render);