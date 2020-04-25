#version 330 core

layout (location = 0) in vec2 aPos;

out vec2 TexCoord;

void main() {
    TexCoord = aPos;
    if (TexCoord.x < 0.0) TexCoord.x = 0.0;
    if (TexCoord.y < 0.0) TexCoord.y = 0.0;

    gl_Position = vec4(aPos, 0.0, 1.0);
}