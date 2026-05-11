#include "../logger.h"
#include "../../globals.h"
#include "platform_api.h"
#include "platform_public.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


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

MAPI Vertex* m_loadFBX(const char* path, size_t* out_count) {
        const struct aiScene* scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                mErr("Failed to load FBX.");
                return NULL;
        }

        size_t total_verts = 0;
        for (unsigned int i=0; i<scene->mNumMeshes; i++) {
                total_verts += scene->mMeshes[i]->mNumFaces * 3;
        }

        Vertex* verts = malloc(sizeof(Vertex) * total_verts);
        if (!verts) {
                mErr("Failed to alloc vertex buffer");
                aiReleaseImport(scene);
                return NULL;
        }

        size_t vi = 0;
        for (unsigned int i=0; i<scene->mNumMeshes; i++) {
                struct aiMesh* mesh = scene->mMeshes[i];

                for (unsigned int j=0; j<mesh->mNumFaces; j++) {
                        struct aiFace face = mesh->mFaces[j];

                        for (unsigned int k=0; k<3; k++) {
                                unsigned int idx = face.mIndices[k];
                                Vertex v = {0};

                v.x = mesh->mVertices[idx].x;
                v.y = mesh->mVertices[idx].y;
                v.z = mesh->mVertices[idx].z;

                if (mesh->mNormals) {
                    v.nx = mesh->mNormals[idx].x;
                    v.ny = mesh->mNormals[idx].y;
                    v.nz = mesh->mNormals[idx].z;
                }

                if (mesh->mTextureCoords[0]) {
                    v.u = mesh->mTextureCoords[0][idx].x;
                    v.v = mesh->mTextureCoords[0][idx].y;
                }

                verts[vi++] = v;
                }
                }
        }

        aiReleaseImport(scene);
        *out_count = total_verts;
        return verts;
}

MAPI Model m_loadModel(const char* path) {
        Model result = {0};

        size_t vert_count = 0;
        Vertex* verts = m_loadFBX(path, &vert_count);
        if (!verts) {
                mInfo("Failed to load FBX.");
                return result;
        } 
        SDL_GPUBufferCreateInfo vbo_info = {
                .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                .size = sizeof(Vertex) * vert_count
        };
        result.vbo = SDL_CreateGPUBuffer(render.device, &vbo_info);

        SDL_GPUTransferBufferCreateInfo transfer_info = {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = sizeof(Vertex) * vert_count
        };
        SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(render.device, &transfer_info);

        Vertex* mapped = SDL_MapGPUTransferBuffer(render.device, transfer, false);
        memcpy(mapped, verts, sizeof(Vertex) * vert_count);
        SDL_UnmapGPUTransferBuffer(render.device, transfer);
        free(verts);

        SDL_GPUCommandBuffer* upload_cmd = SDL_AcquireGPUCommandBuffer(render.device);
        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(upload_cmd);
        SDL_GPUTransferBufferLocation src = { .transfer_buffer = transfer, .offset = 0 };
        SDL_GPUBufferRegion dst = { .buffer = result.vbo, .offset = 0, .size = sizeof(Vertex) * vert_count };
        SDL_UploadToGPUBuffer(copy_pass, &src, &dst, false);
        SDL_EndGPUCopyPass(copy_pass);
        SDL_SubmitGPUCommandBuffer(upload_cmd);
        SDL_ReleaseGPUTransferBuffer(render.device, transfer);

        result.vert_count = vert_count;
        mDebug("Loaded FBX.");
        return result;
}

MAPI void m_drawModel(Model model, vec3 position) {
        if (render.frameLock == FALSE) {
                mWarn("Frame is currently locked. Check where you are calling m_drawModel.");
                return;
        }

        mat4 model_mat;
        glm_mat4_identity(model_mat);
        glm_translate(model_mat, position);

        mat4 mvp;
        glm_mat4_mul(cam.proj_matrix, cam.view_matrix, mvp);
        glm_mat4_mul(mvp, model_mat, mvp);

        SDL_BindGPUGraphicsPipeline(render.renderPass, render.pipeline);
        SDL_GPUBufferBinding vbo_binding = { .buffer = model.vbo, .offset = 0 };
        SDL_BindGPUVertexBuffers(render.renderPass, 0, &vbo_binding, 1);
        SDL_PushGPUVertexUniformData(render.buffer, 0, mvp, sizeof(mat4));
        SDL_DrawGPUPrimitives(render.renderPass, model.vert_count, 1, 0, 0);
}

MAPI void m_freeModel(Model model) {
        SDL_ReleaseGPUBuffer(render.device, model.vbo);
}
