#version 330 core
layout (location = 0) in vec3 a_Pos;

out vec4 TexCoord;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    TexCoord = vec4(a_Pos, 1.0);
    gl_Position = u_Projection * u_View * vec4(a_Pos, 1.0);
    gl_Position = gl_Position.xyww;
}