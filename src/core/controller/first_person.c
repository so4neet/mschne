#include "../../globals.h"
#include "../platform/platform_api.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

void UpdatePlayerPos(Camera3D *cam, float delta_time) {
        // Direction vectors
        float speed = DEF_MOVE_SPEED * delta_time;
        int numkeys;
        const bool *keys = SDL_GetKeyboardState(&numkeys);
        float radYaw = cam->yaw * (3.14159265 / 180.0f);
        float fwX = sinf(radYaw);
        float fwZ = -cosf(radYaw);
        float rX = cosf(radYaw);
        float rZ = sinf(radYaw);

        if(keys[SDL_SCANCODE_A]){
                cam->position[0] += fwX * speed;
                cam->position[2] += fwZ * speed;
        }
        if(keys[SDL_SCANCODE_D]){
                cam->position[0] -= fwX * speed;
                cam->position[2] -= fwZ * speed;
        }
        if(keys[SDL_SCANCODE_S]){
                cam->position[0] -= rX * speed;
                cam->position[2] -= rZ * speed;
        }
        if(keys[SDL_SCANCODE_W]){
                cam->position[0] += rX * speed;
                cam->position[2] += rZ * speed;
        }
        glm_vec3_add(cam->position, cam->direction, cam->target);
        glm_lookat(cam->position, cam->target, cam->up, cam->view_matrix);
}

void HandleFPSMouse(SDL_Event *event, Camera3D *cam) {
        switch (event->type) {
                case SDL_EVENT_MOUSE_MOTION:
                        float sensitivity = 0.1f;
                        float xoffset = event->motion.xrel * sensitivity;
                        float yoffset = -event->motion.yrel * sensitivity;
                        Camera3D_Update(cam, xoffset, yoffset);
                        break;
                default:
                        break;
        }
}