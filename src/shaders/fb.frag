#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D u_ScreenTexture;

void main() {
    vec3 color = texture(u_ScreenTexture, TexCoords).rgb;
    FragColor = vec4(color, 1.0);

    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    //float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
    //FragColor = vec4(vec3(average), 1.0);
}