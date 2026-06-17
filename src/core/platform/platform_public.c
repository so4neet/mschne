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

static const char* CUBE_FACES[6] = {
        "assets/skybox/px.png",
        "assets/skybox/nx.png",
        "assets/skybox/py.png",
        "assets/skybox/ny.png",
        "assets/skybox/pz.png",
        "assets/skybox/nz.png"
};

MAPI void m_loadCubemap() {
        SDL_Surface *faces[6];
        for (int i=0; i<6; i++) {
                SDL_Surface *raw = IMG_Load(CUBE_FACES[i]);
                if (!raw) mErr("Failed to load cubemap face: %s.", CUBE_FACES[i]);
                faces[i] = SDL_ConvertSurface(raw, SDL_PIXELFORMAT_RGBA32);
                SDL_DestroySurface(raw);
        }

        int w = faces[0]->w;
        int h = faces[0]->h;
        size_t faceSize = w * h * 4;

        SDL_GPUTextureCreateInfo cube_info = {
                .type = SDL_GPU_TEXTURETYPE_CUBE,
                .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                .width = w,
                .height = h,
                .layer_count_or_depth = 6,
                .num_levels = 1
        };

        SDL_GPUSamplerCreateInfo samp_info = {
                .min_filter = SDL_GPU_FILTER_LINEAR,
                .mag_filter = SDL_GPU_FILTER_LINEAR,
                .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
                .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        };
        render.cubeSampler = SDL_CreateGPUSampler(render.device, &samp_info);

        SDL_GPUTexture *cubemap = SDL_CreateGPUTexture(render.device, &cube_info);
        if (!cubemap) mErr("Failed to create cubemap GPU texture.");

        SDL_GPUTransferBufferCreateInfo tbuf_info = {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = faceSize * 6
        };
        SDL_GPUTransferBuffer *tbuf = SDL_CreateGPUTransferBuffer(render.device, &tbuf_info);
        uint8_t *mapped = SDL_MapGPUTransferBuffer(render.device, tbuf, false);
        for (int i=0; i<6; i++) {
                memcpy(mapped + faceSize * i, faces[i]->pixels, faceSize);
                SDL_DestroySurface(faces[i]);
        }
        SDL_UnmapGPUTransferBuffer(render.device, tbuf);

        SDL_GPUCommandBuffer *upload_cmd = SDL_AcquireGPUCommandBuffer(render.device);
        SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(upload_cmd);

        for (int i=0; i<6; i++) {
                SDL_GPUTextureTransferInfo src = {
                        .transfer_buffer = tbuf,
                        .offset = faceSize * i
                };
                SDL_GPUTextureRegion dst = {
                        .texture = cubemap,
                        .layer = i,
                        .w = w,
                        .h = h,
                        .d = 1
                };
                SDL_UploadToGPUTexture(copy_pass, &src, &dst, false);
        }

        SDL_EndGPUCopyPass(copy_pass);
        SDL_SubmitGPUCommandBuffer(upload_cmd);
        SDL_ReleaseGPUTransferBuffer(render.device, tbuf);
        mDebug("Loaded skybox.");
        render.cubemap = cubemap;
}

MAPI void m_drawSkybox() {
        if (!render.frameLock) return;

        typedef struct {
                mat4 inv_proj;
                mat4 inv_view;
        } SkyUBO;

        mat4 view_no_trans, inv_proj, inv_view;
        glm_look((vec3){0.0f, 0.0f, 0.0f}, cam.direction, cam.up, view_no_trans);
        glm_mat4_inv(cam.proj_matrix, inv_proj);
        glm_mat4_inv(view_no_trans, inv_view);
        SkyUBO sky_ubo;
        glm_mat4_copy(inv_proj, sky_ubo.inv_proj);
        glm_mat4_copy(inv_view, sky_ubo.inv_view);
        SDL_BindGPUGraphicsPipeline(render.renderPass, render.skyPipeline);
        SDL_GPUTextureSamplerBinding bind = {
                .texture = render.cubemap,
                .sampler = render.cubeSampler
        };
        SDL_BindGPUFragmentSamplers(render.renderPass, 0, &bind, 1);
        SDL_PushGPUFragmentUniformData(render.buffer, 0, &sky_ubo, sizeof(SkyUBO));
        SDL_DrawGPUPrimitives(render.renderPass, 3, 1, 0, 0);
}

MAPI float m_deltaTime() {
        Uint64 ct = SDL_GetTicksNS();
        if (lt == 0) lt = ct;
        float dt = (float)(ct - lt) / 1e9f;
        lt = ct;
        return dt;
}

MAPI b8 m_createWin(app_window win) {
        if (!Plat_InitWindow(win, &render)) {
                // Catch if window can't open and make sure to free memory.
                Plat_FreeSDL(&render);
                return FALSE;
        }
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

static Vertex* m_loadMesh(const char* path, size_t* out_count) {
        b8 is_gltf = strstr(path, ".gltf") || (strstr(path, ".glb"));
        const struct aiScene* scene = aiImportFile(path,
                aiProcess_Triangulate |
                (is_gltf ? 0 : aiProcess_FlipUVs) |
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

static SDL_Surface* load_material_texture(const struct aiScene* scene, struct aiMaterial* mat) {
    struct aiString tex_path_ai;
    if (aiGetMaterialString(mat, AI_MATKEY_TEXTURE_DIFFUSE(0), &tex_path_ai) == AI_SUCCESS) {
        if (tex_path_ai.data[0] == '*') {
            int tex_index = atoi(tex_path_ai.data+1);
            const struct aiTexture* embedded = scene->mTextures[tex_index];
            if (embedded->mHeight == 0) {
                SDL_IOStream* io = SDL_IOFromConstMem(embedded->pcData, embedded->mWidth);
                return IMG_Load_IO(io, 1);
            } else {
                SDL_Surface* s = SDL_CreateSurface(embedded->mWidth, embedded->mHeight, SDL_PIXELFORMAT_RGBA32);
                memcpy(s->pixels, embedded->pcData, embedded->mWidth * embedded->mHeight * 4);
                return s;
            }
        } else {
            return IMG_Load(tex_path_ai.data);
        }
    }
    return NULL;
}

MAPI Model m_loadModel(const char* path, const char* tex_path) {
    Model result = {0};
    glm_vec3_fill(result.aabb_min,  FLT_MAX);
    glm_vec3_fill(result.aabb_max, -FLT_MAX);

    const struct aiScene* scene = aiImportFile(path,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices
    );
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        mErr("Failed to load model: %s", path);
        return result;
    }

    for (unsigned int m = 0; m < scene->mNumMeshes && m < 256; m++) {
        struct aiMesh* mesh = scene->mMeshes[m];
        SubMesh* sub = &result.meshes[result.mesh_count++];

        size_t vert_count = mesh->mNumFaces * 3;
        Vertex* verts = malloc(sizeof(Vertex) * vert_count);
        size_t vi = 0;
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

                // AABB
                glm_vec3_minv(result.aabb_min, (vec3){v.x, v.y, v.z}, result.aabb_min);
                glm_vec3_maxv(result.aabb_max, (vec3){v.x, v.y, v.z}, result.aabb_max);
            }
        }
        sub->vert_count = vert_count;

        SDL_GPUBufferCreateInfo vbo_info = {
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size  = sizeof(Vertex) * vert_count
        };
        sub->vbo = SDL_CreateGPUBuffer(render.device, &vbo_info);
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
        SDL_GPUTransferBufferLocation vsrc = { .transfer_buffer = transfer, .offset = 0 };
        SDL_GPUBufferRegion vdst = { .buffer = sub->vbo, .offset = 0, .size = sizeof(Vertex) * vert_count };
        SDL_UploadToGPUBuffer(copy_pass, &vsrc, &vdst, false);
        SDL_EndGPUCopyPass(copy_pass);
        SDL_SubmitGPUCommandBuffer(upload_cmd);
        SDL_ReleaseGPUTransferBuffer(render.device, transfer);

        SDL_Surface* surface = NULL;
        if (mesh->mMaterialIndex < scene->mNumMaterials) {
            surface = load_material_texture(scene, scene->mMaterials[mesh->mMaterialIndex]);
        }
        if (!surface && tex_path) surface = IMG_Load(tex_path);
        if (!surface) {
            mWarn("Couldn't load texture %s", tex_path);
            surface = IMG_Load("assets/textures/missing.png");
        }
        if (!surface) {
            mFatal("Couldn't load default texture. Skipping.");
            return result;
        }

        SDL_Surface* converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
        SDL_DestroySurface(surface);
        int tw = converted->w, th = converted->h;
        SDL_GPUTextureCreateInfo tex_info = {
            .type                 = SDL_GPU_TEXTURETYPE_2D,
            .format               = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            .usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER,
            .width                = tw, .height = th,
            .layer_count_or_depth = 1, .num_levels = 1
        };
        sub->texture = SDL_CreateGPUTexture(render.device, &tex_info);
        SDL_GPUTransferBufferCreateInfo tex_transfer_info = {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size  = tw * th * 4
        };
        SDL_GPUTransferBuffer* tex_transfer = SDL_CreateGPUTransferBuffer(render.device, &tex_transfer_info);
        void* tex_mapped = SDL_MapGPUTransferBuffer(render.device, tex_transfer, false);
        memcpy(tex_mapped, converted->pixels, tw * th * 4);
        SDL_UnmapGPUTransferBuffer(render.device, tex_transfer);
        SDL_DestroySurface(converted);
        SDL_GPUCommandBuffer* tex_cmd = SDL_AcquireGPUCommandBuffer(render.device);
        SDL_GPUCopyPass* tex_copy = SDL_BeginGPUCopyPass(tex_cmd);
        SDL_GPUTextureTransferInfo tex_src = { .transfer_buffer = tex_transfer, .offset = 0 };
        SDL_GPUTextureRegion tex_dst = { .texture = sub->texture, .w = tw, .h = th, .d = 1 };
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
        sub->sampler = SDL_CreateGPUSampler(render.device, &sampler_info);
    }

    aiReleaseImport(scene);
    mDebug("Loaded model: %s (%d meshes)", path, result.mesh_count);
    return result;
}

MAPI void m_drawModel(Model model, vec3 position, vec3 rotation) {
    if (!render.frameLock) return;

    mat4 model_mat, viewProj, mvp;
    vec4 frust_planes[6];

    glm_mat4_mul(cam.proj_matrix, cam.view_matrix, viewProj);
    glm_frustum_planes(viewProj, frust_planes);
    glm_mat4_identity(model_mat);
    glm_translate(model_mat, position);
    glm_rotate(model_mat, glm_rad(rotation[0]), (vec3){1.0f, 0.0f, 0.0f});
    glm_rotate(model_mat, glm_rad(rotation[1]), (vec3){0.0f, 1.0f, 0.0f});
    glm_rotate(model_mat, glm_rad(rotation[2]), (vec3){0.0f, 0.0f, 1.0f});

    glm_mat4_mul(cam.proj_matrix, cam.view_matrix, mvp);
    glm_mat4_mul(mvp, model_mat, mvp);

    // Frustum cull against whole model AABB
    vec3 local_aabb[2];
    glm_vec3_copy(model.aabb_min, local_aabb[0]);
    glm_vec3_copy(model.aabb_max, local_aabb[1]);
    vec3 world_aabb[2];
    glm_aabb_transform(local_aabb, model_mat, world_aabb);
    if (!glm_aabb_frustum(world_aabb, frust_planes)) return;

    SDL_BindGPUGraphicsPipeline(render.renderPass, render.pipeline);
    SDL_PushGPUVertexUniformData(render.buffer, 0, mvp, sizeof(mat4));

    for (int i = 0; i < model.mesh_count; i++) {
        SubMesh* sub = &model.meshes[i];
        if (!sub->vert_count || !sub->texture) continue;

        SDL_GPUBufferBinding vbo_binding = { .buffer = sub->vbo, .offset = 0 };
        SDL_BindGPUVertexBuffers(render.renderPass, 0, &vbo_binding, 1);
        SDL_GPUTextureSamplerBinding tex_bind = { .texture = sub->texture, .sampler = sub->sampler };
        SDL_BindGPUFragmentSamplers(render.renderPass, 0, &tex_bind, 1);
        SDL_DrawGPUPrimitives(render.renderPass, sub->vert_count, 1, 0, 0);
    }
}

MAPI void m_freeModel(Model model) {
    for (int i = 0; i < model.mesh_count; i++) {
        SubMesh* sub = &model.meshes[i];
        if (sub->vbo)     SDL_ReleaseGPUBuffer(render.device, sub->vbo);
        if (sub->texture) SDL_ReleaseGPUTexture(render.device, sub->texture);
        if (sub->sampler) SDL_ReleaseGPUSampler(render.device, sub->sampler);
    }
}