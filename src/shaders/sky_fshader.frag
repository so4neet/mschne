#version 450

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform samplerCube cubeSampler;
layout(set = 3, binding = 0) uniform SkyUBO {
        mat4 inv_proj;
        mat4 inv_view;
} ubo;

void main() {
        vec4 clip = vec4(inUV.x, inUV.y, 1.0, 1.0);
        vec4 view_space = ubo.inv_proj * clip;
        view_space = vec4(view_space.x, view_space.y, -1.0, 0.0);
        vec3 rayDir = normalize((ubo.inv_view * view_space).xyz);
        outColor = texture(cubeSampler, rayDir);
}