#pragma once
#include <stdio.h>
#include <cglm/cglm.h>
#include <SDL3/SDL.h>
// Global defines and variables for MSCHNE

#if defined(_WIN32)
        #define MAPI __declspec(dllexport)
#else
        #define MAPI __attribute__((visibility("default")))
#endif

// SDL
#define SDL_FLAGS SDL_INIT_VIDEO

// Data types

typedef int b8;
#define TRUE            1
#define FALSE           0

// Window defs

#define DEF_WIDTH       1280
#define DEF_HEIGHT      720
#define DEF_WIN_NAME    "MSCHNE Window"
#define DEF_FOV         75.0f

// Shader defs
// TODO: Switch to SDL_Shadercross
#define DEF_VSHADER_PATH        "assets/shaders/vshader.spv"
#define DEF_FSHADER_PATH        "assets/shaders/fshader.spv"
#define DEF_SKYBOX_VSHADER_PATH     "assets/shaders/sky_vshader.spv"
#define DEF_SKYBOX_FSHADER_PATH     "assets/shaders/sky_fshader.spv"

// Movement defs
#define DEF_MOVE_SPEED  4.0f

// Window structs

typedef struct app_window {
        int width;
        int height;
        const char* winName;
        float fov;
} app_window;

// Engine structs

typedef struct renderer {
        SDL_Window *window;
        SDL_GPUDevice *device;
        SDL_GPUCommandBuffer *buffer;
        SDL_GPUGraphicsPipeline *pipeline;
        SDL_GPUGraphicsPipeline *skyPipeline;
        SDL_GPURenderPass *renderPass;
        SDL_GPUTexture *cubemap;
        SDL_GPUSampler *cubeSampler;
        SDL_GPUTexture *swapchain;
        SDL_GPUTexture *depthTex;
        SDL_GPUBuffer *vbo;
        SDL_GPUBuffer *ibo;
        SDL_GPUShader *vshader;
        SDL_GPUShader *fshader;
        SDL_GPUShader *vshaderSky;
        SDL_GPUShader *fshaderSky;
        SDL_Event event;
        b8 isRunning;
        b8 frameLock;
} renderer;

typedef struct Camera3D {
        vec3 position;
        vec3 target;
        vec3 direction;
        vec3 up;
        vec3 camRight;
        vec3 camUp;
        mat4 view_matrix;
        mat4 proj_matrix;
        float pitch;
        float yaw;
} Camera3D;

typedef struct Vertex {
        float x,y,z;
        float nx, ny, nz;
        float u, v;
} Vertex;

typedef struct Model {
        SDL_GPUBuffer* vbo;
        SDL_GPUTexture* texture;
        SDL_GPUSampler* sampler;
        size_t vert_count;
        vec3 aabb_min;
        vec3 aabb_max;
} Model;

typedef struct RawModel {
        Vertex* verts;
        size_t vert_count;
        void* tex_data;
        int tex_width;
        int tex_height;
} RawModel;

typedef struct StaticObject {
        Model model;
        vec3 pos;
} StaticObject;
