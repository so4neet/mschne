#version 450

vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
);

layout(set = 0, binding = 0) uniform SkyUBO {
        mat4 inv_view_proj;
} ubo;

layout(location = 0) out vec3 outRayDir;

void main() {
        vec2 pos = positions[gl_VertexIndex];
        gl_Position = vec4(pos, 1.0, 1.0);
        vec4 ray = ubo.inv_view_proj * vec4(pos, 0.0, 1.0);
        outRayDir = normalize(ray.xyz / ray.w);
}