#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
//uniform sampler2D container;
//uniform sampler2D smiley;

uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = vec4(lightColor * objectColor, 1.0f);
}
