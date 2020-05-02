#version 330 core

layout (location = 0) in vec3 a_CSPos;

uniform mat4 u_Projection;

void main() {
    gl_Position = u_Projection * vec4(a_CSPos, 1.0);
}