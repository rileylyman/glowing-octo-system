#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D container;
uniform sampler2D smiley;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = mix(texture(container, TexCoord), texture(smiley, TexCoord), 0.2);
}
