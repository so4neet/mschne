#include "../logger.h"
#include "../../globals.h"
#include "platform_api.h"
#include "platform_public.h"

renderer render = {0};
Camera3D cam = {0};

MAPI b8 m_createWin(app_window win) {
        Plat_InitWindow(win, &render);
        Camera3D_Init(&cam, &win);
        return TRUE;
}

MAPI b8 m_pollEvents(){
        return Plat_Event(&render);
}

MAPI void m_startFrame() {
        Plat_StartFrame(&render);
}

MAPI void m_endFrame() {
        Plat_EndFrame(&render);
}

MAPI void m_destroyWin() {
        Plat_FreeSDL(&render);
}

MAPI void m_updateCam() {
        Camera3D_Update(&cam);
}

MAPI void m_drawTestCube(vec3 position) {
        if (render.frameLock == FALSE) {
                mWarn("Frame is currently locked. Check where you are calling m_drawTestCube.");
                return;
        }

        mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, position);

        mat4 mvp;
        glm_mat4_mul(cam.proj_matrix, cam.view_matrix, mvp);
        glm_mat4_mul(mvp, model, mvp);

        SDL_BindGPUGraphicsPipeline(render.renderPass, render.pipeline);
        SDL_GPUBufferBinding vbo_binding = {
                .buffer = render.vbo,
                .offset = 0
        };
        SDL_BindGPUVertexBuffers(render.renderPass, 0,  &vbo_binding, 1);
        SDL_GPUBufferBinding ibo_binding = {
                .buffer = render.ibo,
                .offset = 0
        };
        SDL_BindGPUIndexBuffer(render.renderPass, &ibo_binding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        SDL_PushGPUVertexUniformData(render.buffer, 0, mvp, sizeof(mat4));
        SDL_DrawGPUIndexedPrimitives(render.renderPass, 36, 1, 0, 0, 0);
}
