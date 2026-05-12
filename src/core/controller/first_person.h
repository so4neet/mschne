#pragma once
#include "../../globals.h"
#include <SDL3/SDL.h>

void HandleFPSMouse(SDL_Event* event, Camera3D *cam);
void UpdatePlayerPos(Camera3D *cam, float delta_time);