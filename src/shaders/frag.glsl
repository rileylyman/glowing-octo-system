#version 330 core

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in Light light;

//uniform sampler2D container;
//uniform sampler2D smiley;

void main()
{
	// Ambient
	vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;

	// Diffuse
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(normal, lightDir),  0.0);
	vec3 diffuse = diff * texture(material.diffuse, TexCoord).rgb * light.diffuse;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(-FragPos);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = spec * texture(material.specular, TexCoord).rgb * light.specular;

	vec3 result = ambient + diffuse + specular;

	FragColor = vec4(result, 1.0);
}
