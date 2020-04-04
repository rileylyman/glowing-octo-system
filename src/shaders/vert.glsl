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
out mat3 TBN;

uniform mat4 transform;
uniform mat4 model;
uniform mat3 normal_matrix;
uniform vec3 camera_pos;

void main()
{
	vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
	vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
	Normal = normalize(vec3(model * vec4(aNormal, 0.0))); 

	TBN = mat3(T, B, Normal);

	gl_Position = transform * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
	FragPos = vec3(model * vec4(aPos, 1.0));
	CameraPos = camera_pos;
}
