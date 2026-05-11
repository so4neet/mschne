#version 450

layout(set = 1, binding = 0) uniform UBO {
    mat4 mvp;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec4 fragColor; 

void main() {
    gl_Position = mvp * vec4(inPosition, 1.0);
    fragColor = vec4(inColor, 1.0); 
}
