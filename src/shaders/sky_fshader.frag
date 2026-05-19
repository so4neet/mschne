#version 450

layout(location = 0) in vec3 inRayDir;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform samplerCube cubeSampler;

void main() {
        outColor = texture(cubeSampler, normalize(inRayDir));
}