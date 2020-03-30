#version 330 core

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define MAX_NR_DIRLIGHTS 1
uniform DirLight u_DirLights[MAX_NR_DIRLIGHTS];
uniform int u_NrDirLights = 0;

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
#define MAX_NR_POINTLIGHTS 16
uniform PointLight u_PointLights[MAX_NR_POINTLIGHTS];
uniform int u_NrPointLights = 0;

struct Spotlight {
	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	float cosPhi;
	float cosGamma;
};
#define MAX_NR_SPOTLIGHTS 1
uniform Spotlight u_Spotlights[MAX_NR_SPOTLIGHTS];
uniform int u_NrSpotlights = 0;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec3 CameraPos;

out vec4 FragColor;

vec3 CalculateDirLight(DirLight light) {
	vec3 lightDir = normalize(-light.direction);

	// Ambient
	vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;

	// Diffuse
	vec3 normal = normalize(Normal);
	float diff = max(dot(normal, lightDir),  0.0);
	vec3 diffuse = diff * texture(material.diffuse, TexCoord).rgb * light.diffuse;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(CameraPos - FragPos);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = spec * texture(material.specular, TexCoord).rgb * light.specular;

	vec3 result = ambient + diffuse + specular;
	return result;
}

vec3 CalculatePointLight(PointLight light) {
	vec3 lightDir = normalize(light.position - FragPos);

	// Ambient
	vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;

	// Diffuse
	vec3 normal = normalize(Normal);
	float diff = max(dot(normal, lightDir),  0.0);
	vec3 diffuse = diff * texture(material.diffuse, TexCoord).rgb * light.diffuse;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(CameraPos - FragPos);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = spec * texture(material.specular, TexCoord).rgb * light.specular;

	float dist = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	vec3 result = attenuation * (ambient + diffuse + specular);
	return result;
}

vec3 CalculateSpotlight(Spotlight light) {
	vec3 lightDir = normalize(light.position - FragPos);

	// Ambient
	vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;

	// Diffuse
	vec3 normal = normalize(Normal);
	float diff = max(dot(normal, lightDir),  0.0);
	vec3 diffuse = diff * texture(material.diffuse, TexCoord).rgb * light.diffuse;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(CameraPos - FragPos);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = spec * texture(material.specular, TexCoord).rgb * light.specular;

	float distance = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	float cosTheta = dot(lightDir, normalize(-light.direction));
	float intensity = 1.0;
	if (cosTheta <= light.cosPhi && cosTheta > light.cosGamma) {
		float epsilon = light.cosPhi - light.cosGamma;
		intensity = (cosTheta - light.cosGamma) / epsilon;
	} else if (cosTheta < light.cosGamma) {
		intensity = 0.0;
	}

	vec3 result = attenuation * (ambient + intensity * (diffuse + specular));
	return result;
}

void main()
{
	vec3 result = vec3(0.0);
	for (int i = 0; i < u_NrDirLights; i++) {
		result += CalculateDirLight(u_DirLights[i]);
	}
	for (int i = 0; i < u_NrPointLights; i++) {
		result += CalculatePointLight(u_PointLights[i]);
	}
	for (int i = 0; i < u_NrSpotlights; i++) {
		result += CalculateSpotlight(u_Spotlights[i]);
	}

//	FragColor = vec4(debug_color, 1.0);
	FragColor = vec4(result, 1.0);
}
