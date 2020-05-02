#version 330 core

layout (location = 0) in vec3 a_CSPos;

uniform mat4 u_Projection;
uniform mat4 u_InverseView;

out vec3 WSPosition;

void main() {
    WSPosition = vec3(u_InverseView * vec4(a_CSPos, 1.0));
    gl_Position = u_Projection * vec4(a_CSPos, 1.0);
}