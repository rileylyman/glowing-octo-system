#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform Light lightU;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out Light light;

uniform mat4 transform;
uniform mat4 model_view;
uniform mat3 normal_matrix_view;
uniform mat4 view;

void main()
{
	light = lightU;
	light.position = vec3(view * vec4(lightU.position, 1.0));

	gl_Position = transform * vec4(aPos, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
	Normal = normal_matrix_view * aNormal; 
	FragPos = vec3(model_view * vec4(aPos, 1.0));
}
