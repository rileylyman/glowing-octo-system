#version 330 core

out vec4 FragColor;

uniform sampler3D u_Grid0;
uniform sampler3D u_Grid1;

uniform float u_Depth;

in vec2 TexCoord;

void main() {

    vec3 value_0 = texture(u_Grid0, vec3(TexCoord, u_Depth)).rgb;
    vec3 value_1 = texture(u_Grid1, vec3(TexCoord, u_Depth)).rgb;

    FragColor = vec4(value_0 + value_1, 1.0);
}