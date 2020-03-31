#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aBitangent;
layout (location = 4) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec3 CameraPos;

uniform mat4 transform;
uniform mat4 model;
uniform mat3 normal_matrix;
uniform vec3 camera_pos;

void main()
{
	gl_Position = transform * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
	Normal = normal_matrix * aNormal; 
	FragPos = vec3(model * vec4(aPos, 1.0));
	CameraPos = camera_pos;
}
