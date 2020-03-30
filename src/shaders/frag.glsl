#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec3 LightPos;

// texture samplers
//uniform sampler2D container;
//uniform sampler2D smiley;

uniform vec3 lightColor;
uniform vec3 objectColor;

float ambientStrength = 0.1;
float specularStrength = 0.5;

void main()
{
	// Ambient
	vec3 ambient = ambientStrength * lightColor;

	// Diffuse
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(LightPos - FragPos);
	float diff = max(dot(normal, lightDir),  0.0);
	vec3 diffuse = diff * lightColor;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(-FragPos);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = spec * specularStrength * lightColor;

	FragColor = vec4((ambient + diffuse + specular) * objectColor, 1.0);
}
