#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include "../logger.h"
#include "../../globals.h"
#include "platform_linux.h"

// Most of this code was created by following "Open Window - Beginners Guide to SDL3 in C - Part 1 | Programming Rainbow"
// TODO: Change structure from platform_common and platform_linux to something else, SDL handles platform management

void Plat_FreeSDL(renderer *render) {
        if(render->device) {
                SDL_DestroyGPUDevice(render->device);
        }
        if(render->window) {
                SDL_DestroyWindow(render->window);
                mInfo("Destroyed window.");
                render->window = NULL;
        }


        SDL_Quit();
}

b8 Plat_Event(renderer *render) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
                switch (event.type) {
                        case SDL_EVENT_QUIT:
                                render->isRunning = false;
                                break;
                        default:
                                break;
                }
        }
        return render->isRunning;
}

void Plat_Draw(renderer *render) {
        // Create the command buffer
        render->buffer = SDL_AcquireGPUCommandBuffer(render->device);

        // Acquire swapchain texture
        SDL_WaitAndAcquireGPUSwapchainTexture(render->buffer, render->window, &render->swapchain, NULL, NULL);
        if (!render->swapchain) {
                // End frame if swapchain buffer isn't available
                SDL_SubmitGPUCommandBuffer(render->buffer);
        }

        SDL_GPUColorTargetInfo colorTargetInfo = {0};
        colorTargetInfo.clear_color = (SDL_FColor) {0.3f, 0.6f, 0.5f, 1.0f};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        colorTargetInfo.texture = render->swapchain;

        // Render pass
        render->renderPass = SDL_BeginGPURenderPass(render->buffer, &colorTargetInfo, 1, NULL);
        SDL_EndGPURenderPass(render->renderPass);
        SDL_SubmitGPUCommandBuffer(render->buffer);
}

b8 Plat_InitWindow(app_window win, renderer *render) {
        // Initialize SDL
        if (!SDL_Init(SDL_FLAGS)) {
                mFatal("Couldn't init renderer, aborting.");
                return FALSE;
        }
        // Check if w,h,n are defined
        if (win.width == 0) {
                mWarn("Window width not defined, defaulting...");
                win.width = DEF_WIDTH;
        }
        if (win.height == 0) {
                mWarn("Window height not defined, defaulting...");
                win.height = DEF_HEIGHT;
        }
        if (win.winName == NULL) {
                mWarn("Window name not defined, defaulting...");
                win.winName = DEF_WIN_NAME;
        }

        // Initialize the GPU 
        render->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, true, NULL);
        if (!render->device) {
                mFatal("Couldn't init GPU, aborting.");
                return FALSE;
        } else {
                mInfo("Initialized GPU.");
        }

        // Create the window
        render->window = SDL_CreateWindow(win.winName, win.width, win.height, 0);
        if (!render->window) {
                mFatal("Couldn't create window, aborting.");
                return FALSE;
        } else {
                mInfo("Initialized window.");
        }

        // Claim the window for the GPU
        if (!SDL_ClaimWindowForGPUDevice(render->device, render->window)) {
                mFatal("Couldn't claim window, aborting.");
                return FALSE;
        } else {
                mInfo("Claimed window.");
        }
        render->isRunning = TRUE;
        mInfo("Opened window.");
        return TRUE;
}

