#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec3 LightPos;

uniform mat4 transform;
uniform mat4 model_view;
uniform mat3 normal_matrix_view;
uniform mat4 view;

uniform vec3 lightPos;

void main()
{
	gl_Position = transform * vec4(aPos, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
	Normal = normal_matrix_view * aNormal; 
	LightPos = vec3(view * vec4(lightPos, 1.0));
	FragPos = vec3(model_view * vec4(aPos, 1.0));
}
