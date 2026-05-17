#include "../logger.h"
#include "../../globals.h"
#include "platform_api.h"
#include "platform_public.h"
#include "../controller/first_person.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <SDL3_image/SDL_image.h>

static Uint64 lt = 0;
static float delta_time = 0.0f;
renderer render = {0};
Camera3D cam = {0};

MAPI float m_deltaTime() {
        Uint64 ct = SDL_GetTicksNS();
        if (lt == 0) lt = ct;
        float dt = (float)(ct - lt) / 1e9f;
        lt = ct;
        return dt;
}

MAPI b8 m_createWin(app_window win) {
        Plat_InitWindow(win, &render);
        Camera3D_Init(&cam, &win);
        return TRUE;
}

MAPI b8 m_pollEvents(){
        delta_time = m_deltaTime();
        b8 running = Plat_Event(&render, &cam);
        UpdatePlayerPos(&cam, delta_time);
        return running;
}

MAPI void m_startFrame() {
        Plat_StartFrame(&render);
}

MAPI void m_endFrame() {
        Plat_EndFrame(&render, &cam);
}

MAPI void m_destroyWin() {
        Plat_FreeSDL(&render);
}

static Vertex* m_loadFBX(const char* path, size_t* out_count) {
        const struct aiScene* scene = aiImportFile(path,
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_GenNormals |
                aiProcess_JoinIdenticalVertices
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                return NULL;
        }

        size_t total_verts = 0;
        for (unsigned int i = 0; i < scene->mNumMeshes; i++)
                total_verts += scene->mMeshes[i]->mNumFaces * 3;

        Vertex* verts = malloc(sizeof(Vertex) * total_verts);
        if (!verts) {
                mErr("Failed to alloc vertex buffer");
                aiReleaseImport(scene);
                return NULL;
        }

        size_t vi = 0;
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
                struct aiMesh* mesh = scene->mMeshes[i];
                for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
                        struct aiFace face = mesh->mFaces[j];
                        for (unsigned int k = 0; k < 3; k++) {
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

MAPI Model m_loadModel(const char* path, const char* tex_path) {
        Model result = {0};
        size_t vert_count = 0;
        Vertex* verts = m_loadFBX(path, &vert_count);
        if (!verts) {
                mErr("Failed to load %s", path);
                return result;
        }
        // Generate AABB
        glm_vec3_fill(result.aabb_min, FLT_MAX);
        glm_vec3_fill(result.aabb_max, -FLT_MAX);
        for (size_t i=0; i<vert_count; i++) {
                glm_vec3_minv(result.aabb_min, (vec3){verts[i].x, verts[i].y, verts[i].z}, result.aabb_min);
                glm_vec3_maxv(result.aabb_max, (vec3){verts[i].x, verts[i].y, verts[i].z}, result.aabb_max);
        }
        SDL_GPUBufferCreateInfo vbo_info = {
                .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                .size  = sizeof(Vertex) * vert_count
        };
        result.vbo = SDL_CreateGPUBuffer(render.device, &vbo_info);

        SDL_GPUTransferBufferCreateInfo transfer_info = {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size  = sizeof(Vertex) * vert_count
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
        SDL_Surface* surface = IMG_Load(tex_path);
        if (!surface) {
                mErr("Failed to load texture, defaulting to error texture.");
                surface = IMG_Load("assets/textures/missing.png");
                if (!surface) {
                        mFatal("Failed to load warning texture, skipping model.");
                        return result;
                }
        }

        SDL_Surface* converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
        SDL_DestroySurface(surface);

        int tex_w = converted->w;
        int tex_h = converted->h;
        size_t tex_size = tex_w * tex_h * 4;

        SDL_GPUTextureCreateInfo tex_info = {
                .type                 = SDL_GPU_TEXTURETYPE_2D,
                .format               = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                .usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                .width                = converted->w,
                .height               = converted->h,
                .layer_count_or_depth = 1,
                .num_levels           = 1
        };
        result.texture = SDL_CreateGPUTexture(render.device, &tex_info);

        SDL_GPUTransferBufferCreateInfo tex_transfer_info = {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size  = converted->w * converted->h * 4
        };
        SDL_GPUTransferBuffer* tex_transfer = SDL_CreateGPUTransferBuffer(render.device, &tex_transfer_info);
        void* tex_mapped = SDL_MapGPUTransferBuffer(render.device, tex_transfer, false);
        memcpy(tex_mapped, converted->pixels, converted->w * converted->h * 4);
        SDL_UnmapGPUTransferBuffer(render.device, tex_transfer);
        SDL_DestroySurface(converted);

        SDL_GPUCommandBuffer* tex_cmd = SDL_AcquireGPUCommandBuffer(render.device);
        SDL_GPUCopyPass* tex_copy = SDL_BeginGPUCopyPass(tex_cmd);
        SDL_GPUTextureTransferInfo tex_src = { .transfer_buffer = tex_transfer, .offset = 0 };
        SDL_GPUTextureRegion tex_dst = {
                .texture = result.texture,
                .w       = tex_w,
                .h       = tex_h,
                .d       = 1
        };
        SDL_UploadToGPUTexture(tex_copy, &tex_src, &tex_dst, false);
        SDL_EndGPUCopyPass(tex_copy);
        SDL_SubmitGPUCommandBuffer(tex_cmd);
        SDL_ReleaseGPUTransferBuffer(render.device, tex_transfer);

        SDL_GPUSamplerCreateInfo sampler_info = {
                .min_filter     = SDL_GPU_FILTER_LINEAR,
                .mag_filter     = SDL_GPU_FILTER_LINEAR,
                .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT
        };
        result.sampler = SDL_CreateGPUSampler(render.device, &sampler_info);
        mDebug("Loaded model: %s", path);
        return result;
}

MAPI void m_drawModel(Model model, vec3 position, vec3 rotation) {
        if (render.frameLock == FALSE) {
                mWarn("Frame is currently locked.");
                return;
        }
        if (!model.vert_count || !model.texture) {
                return;
        }
        mat4 viewProj;
        mat4 model_mat;
        mat4 mvp;
        vec4 frust_planes[6];

        glm_mat4_mul(cam.proj_matrix, cam.view_matrix, viewProj);
        glm_frustum_planes(viewProj, frust_planes);
        glm_mat4_identity(model_mat);
        // Translate and rotate
        glm_translate(model_mat, position);
        glm_rotate(model_mat, glm_rad(rotation[0]), (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(model_mat, glm_rad(rotation[1]), (vec3){0.0f, 1.0f, 0.0f});
        glm_rotate(model_mat, glm_rad(rotation[2]), (vec3){0.0f, 0.0f, 1.0f});
        // Frustum check
        glm_mat4_mul(cam.proj_matrix, cam.view_matrix, mvp);
        glm_mat4_mul(mvp, model_mat, mvp);
        vec3 world_aabb[2], local_aabb[2];
        glm_vec3_copy(model.aabb_min, local_aabb[0]);
        glm_vec3_copy(model.aabb_max, local_aabb[1]);
        glm_aabb_transform(local_aabb, model_mat, world_aabb);
        if (!glm_aabb_frustum(world_aabb, frust_planes)) {
                return;
        }
        SDL_PushGPUVertexUniformData(render.buffer, 0, mvp, sizeof(mat4));
        SDL_BindGPUGraphicsPipeline(render.renderPass, render.pipeline);
        SDL_GPUBufferBinding vbo_binding = { .buffer = model.vbo, .offset = 0 };
        SDL_BindGPUVertexBuffers(render.renderPass, 0, &vbo_binding, 1);
        if (model.texture && model.sampler) {
                SDL_GPUTextureSamplerBinding tex_bind = {
                        .texture = model.texture,
                        .sampler = model.sampler
                };
                SDL_BindGPUFragmentSamplers(render.renderPass, 0, &tex_bind, 1);
        }
        SDL_DrawGPUPrimitives(render.renderPass, model.vert_count, 1, 0, 0);
}

MAPI void m_freeModel(Model model) {
        SDL_ReleaseGPUBuffer(render.device, model.vbo);
        if (model.texture) SDL_ReleaseGPUTexture(render.device, model.texture);
        if (model.sampler) SDL_ReleaseGPUSampler(render.device, model.sampler);
}
