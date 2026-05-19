#pragma once
#include <stdlib.h>
#include "../../globals.h"

b8 Plat_InitWindow(app_window win, renderer *render);
void Plat_FreeSDL(renderer *render);
b8 Plat_Event(renderer *render, Camera3D *cam);
void Plat_StartFrame(renderer *render);
void Plat_EndFrame(renderer *render, Camera3D *cam);
void Camera3D_Update(Camera3D *cam, float xoffset, float yoffset);
void Camera3D_Init(Camera3D *cam, app_window *win);
void Plat_InitBuffers(renderer *render);
static SDL_GPUShader* Plat_LoadShader(renderer *render, const char* path, SDL_GPUShaderStage stage, Uint32 num_uniform_buffers, Uint32 num_samplers);
