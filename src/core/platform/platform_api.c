#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include "../logger.h"
#include "../../globals.h"
#include "platform_api.h"

// References used:
// "Open Window - Beginners Guide to SDL3 in C - Part 1 | Programming Rainbow"
// "Getting started with SDL3_gpu : Clear screen | glusoft.com"

// Debug cube
Vertex cube_verts[] = {
    // position              // color
    {-0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f}, // front face - red
    { 0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f},
    { 0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f},
    {-0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f},
    {-0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f}, // back face - green
    { 0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f},
    { 0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f},
    {-0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f},
};

uint16_t cube_indices[] = {
    // front
    0, 1, 2,  0, 2, 3,
    // back
    5, 4, 7,  5, 7, 6,
    // left
    4, 0, 3,  4, 3, 7,
    // right
    1, 5, 6,  1, 6, 2,
    // top
    3, 2, 6,  3, 6, 7,
    // bottom
    4, 5, 1,  4, 1, 0
};

void Plat_LoadVertexShader(const char* path, renderer *render) {
        size_t size;
        void* vert_shdr = SDL_LoadFile(path, &size);
        if (!vert_shdr) {
                mFatal("Couldn't load vertex shader.");
        }

        SDL_GPUShaderCreateInfo vert_shader_info = {
                .num_uniform_buffers = 1,
                .num_samplers = 0,
                .num_storage_textures = 0,
                .format = SDL_GPU_SHADERFORMAT_SPIRV,
                .stage = SDL_GPU_SHADERSTAGE_VERTEX,
                .entrypoint = "main",
                .code = vert_shdr,
                .code_size = size
        };

        render->vshader = SDL_CreateGPUShader(render->device, &vert_shader_info);
        mInfo("Loaded vertex shader.");
}

void Plat_LoadFragmentShader(const char* path, renderer *render) {
        size_t size;
        void* frag_shdr = SDL_LoadFile(path, &size);
        if (!frag_shdr) {
                mFatal("Couldn't load fragment shader.");
        }

        SDL_GPUShaderCreateInfo frag_shader_info = {
                .num_uniform_buffers = 0,
                .format = SDL_GPU_SHADERFORMAT_SPIRV,
                .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
                .entrypoint = "main",
                .code = frag_shdr,
                .code_size = size
        };

        render->fshader = SDL_CreateGPUShader(render->device, &frag_shader_info);
        mInfo("Loaded fragment shader.");
}


void Plat_FreeSDL(renderer *render) {
        if(render->pipeline) SDL_ReleaseGPUGraphicsPipeline(render->device, render->pipeline);
        if(render->vshader) SDL_ReleaseGPUShader(render->device, render->vshader);
        if(render->fshader) SDL_ReleaseGPUShader(render->device, render->fshader);
        if(render->vbo) SDL_ReleaseGPUBuffer(render->device, render->vbo);
        if(render->ibo) SDL_ReleaseGPUBuffer(render->device, render->ibo);
        if(render->depthTex) SDL_ReleaseGPUTexture(render->device, render->depthTex);
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

void Plat_InitBuffers(renderer *render) {
        uint32_t vert_size = sizeof(cube_verts);
        uint32_t ind_size = sizeof(cube_indices);

        SDL_GPUBufferCreateInfo vbo_desc = {
                .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                .size = vert_size
        };
        render->vbo = SDL_CreateGPUBuffer(render->device, &vbo_desc);

        SDL_GPUBufferCreateInfo ibo_desc = {
                .usage = SDL_GPU_BUFFERUSAGE_INDEX,
                .size = ind_size
        };
        render->ibo = SDL_CreateGPUBuffer(render->device, &ibo_desc);

        SDL_GPUTransferBufferCreateInfo transfer_desc = {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = vert_size + ind_size
        };
        SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(render->device, &transfer_desc);

        if (!transferBuffer) {
                mFatal("Failed to create transfer buffer.");
        }

        void* data = SDL_MapGPUTransferBuffer(render->device, transferBuffer, false);
        if (!data) {
                mFatal("Failed to map transfer buffer.");
        }
        memcpy(data, cube_verts, vert_size);
        memcpy((uint8_t*)data + vert_size, cube_indices, ind_size);

        SDL_UnmapGPUTransferBuffer(render->device, transferBuffer);

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(render->device);
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);

        SDL_UploadToGPUBuffer(copyPass,
        &(SDL_GPUTransferBufferLocation){.transfer_buffer = transferBuffer, .offset = 0},
        &(SDL_GPUBufferRegion){.buffer = render->vbo, .offset = 0, .size = vert_size},
        false);

        SDL_UploadToGPUBuffer(copyPass,
        &(SDL_GPUTransferBufferLocation){.transfer_buffer = transferBuffer, .offset = vert_size},
        &(SDL_GPUBufferRegion){.buffer = render->ibo, .size = ind_size},
        false);

        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(cmd);
        
        SDL_WaitForGPUIdle(render->device);

        SDL_ReleaseGPUTransferBuffer(render->device, transferBuffer);
        mInfo("VBO and IBO Initialized.");
        
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

void Plat_StartFrame(renderer *render) {
        render->buffer = SDL_AcquireGPUCommandBuffer(render->device);

        if (!SDL_WaitAndAcquireGPUSwapchainTexture(render->buffer, render->window, &render->swapchain, NULL, NULL)) {
                SDL_SubmitGPUCommandBuffer(render->buffer);
                mInfo("Couldn't acquire swapchain buffer.");
                render->frameLock = FALSE;
                return;
        }

        SDL_GPUDepthStencilTargetInfo depthTargetInfo = {
                .texture = render->depthTex,
                .clear_depth = 1.0f,
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_DONT_CARE,
                .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
                .cycle = false
        };

        SDL_GPUColorTargetInfo colorTargetInfo = {
                .texture = render->swapchain,
                .clear_color = (SDL_FColor){0.3f, 0.6f, 0.5f, 1.0f},
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE
        };

        render->renderPass = SDL_BeginGPURenderPass(render->buffer, &colorTargetInfo, 1, &depthTargetInfo);
        render->frameLock = TRUE;
}

void Plat_EndFrame(renderer *render) {
        if (!render->frameLock) return;

        SDL_EndGPURenderPass(render->renderPass);
        SDL_SubmitGPUCommandBuffer(render->buffer);

        render->frameLock = FALSE;
        render->renderPass = NULL;
        render->buffer = NULL;
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
        SDL_GPUTextureCreateInfo depth_info = {
                .type = SDL_GPU_TEXTURETYPE_2D,
                .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
                .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
                .width = win.width,
                .height = win.height,
                .layer_count_or_depth = 1,
                .num_levels = 1
        };
        render->depthTex = SDL_CreateGPUTexture(render->device, &depth_info);
        if (!render->depthTex) {
                mFatal("Couldn't create depth texture.");
                return FALSE;
        }
        Plat_InitBuffers(render);
        Plat_LoadVertexShader(DEF_VSHADER_PATH, render);
        Plat_LoadFragmentShader(DEF_FSHADER_PATH, render);
        SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
                .target_info = {
                        .num_color_targets = 1,
                        .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                                .format = SDL_GetGPUSwapchainTextureFormat(render->device, render->window),
                                .blend_state = {
                                        .enable_blend = true,
                                        .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                                        .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                                        .color_blend_op        = SDL_GPU_BLENDOP_ADD,
                                        .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                                        .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                                        .alpha_blend_op       = SDL_GPU_BLENDOP_ADD,
                                        .color_write_mask      = SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G | 
                                                                SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A
                                }
                        }},
                        .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
                        .has_depth_stencil_target = true
                },
                .vertex_input_state = {
                        .num_vertex_buffers = 1,
                        .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]) {{
                                .slot = 0,
                                .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                                .instance_step_rate = 0,
                                .pitch = sizeof(Vertex)
                        }},
                        .num_vertex_attributes = 2,
                        .vertex_attributes = (SDL_GPUVertexAttribute[]) {
                                {
                                .location = 0,
                                .buffer_slot = 0,
                                .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                                .offset = 0
                        },
                        {
                                .location = 1,
                                .buffer_slot = 0,
                                .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                                .offset = sizeof(float) * 3
                        }
                }
                },

                .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,

                .rasterizer_state = {
                        .fill_mode = SDL_GPU_FILLMODE_FILL,
                        .cull_mode = SDL_GPU_CULLMODE_NONE,
                        .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
                },
                
                .depth_stencil_state = {
                        .enable_depth_test = true,
                        .enable_depth_write = true,
                        .compare_op = SDL_GPU_COMPAREOP_LESS
                },
                .vertex_shader = render->vshader,
                .fragment_shader = render->fshader
        };
        render->pipeline = SDL_CreateGPUGraphicsPipeline(render->device, &pipeline_info);
        if (!render->pipeline) {
                mFatal("Couldn't create graphics pipeline.");
                return FALSE;
        }
        render->isRunning = TRUE;
        mInfo("Opened window.");
        return TRUE;
}

void Camera3D_Update(Camera3D *cam) {
        glm_lookat(cam->position, cam->target, cam->camUp, cam->view_matrix);
}

void Camera3D_Init(Camera3D *cam, app_window *win) {
        glm_vec3_copy((vec3){0.0f, 0.0f, 1.0f}, cam->position);
        glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, cam->target);
        glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam->up);
        glm_vec3_sub(cam->position, cam->target, cam->direction);
        glm_vec3_cross(cam->up, cam->direction, cam->camRight);
        glm_vec3_normalize(cam->camRight);
        glm_vec3_cross(cam->direction, cam->camRight, cam->camUp);
        float aspect = (float)win->width / (float)win->height;
        glm_perspective(glm_rad(90.0f), aspect, 0.1f, 100.0f, cam->proj_matrix);
        glm_lookat(cam->position, cam->target, cam->camUp, cam->view_matrix);
}
