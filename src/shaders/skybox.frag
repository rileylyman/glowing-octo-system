#version 330 core

out vec4 FragColor;

in vec4 TexCoord;

uniform samplerCube u_Skybox;

void main() {
    FragColor = texture(u_Skybox, vec3(TexCoord));
}