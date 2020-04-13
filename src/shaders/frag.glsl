#version 330 core

uniform bool u_Reinhard = true;

struct Material {
	sampler2D ambient;
	sampler2D diffuse;
	sampler2D specular;
	sampler2D normal;
	sampler2D height;
	float shininess;
};
uniform Material material;

struct SolidMaterial {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};
uniform SolidMaterial u_SolidMaterial;
uniform bool u_Solid = false;

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define MAX_NR_DIRLIGHTS 8
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
uniform bool u_RenderNormals = true;

in vec2 TexCoord;
mat3 TBN;
in vec3 T;
in vec3 B;
in vec3 Normal;
in vec3 FragPos;
in vec3 CameraPos;

out vec4 FragColor;

vec3 BlinnPhong(vec3 lightDir, vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular, vec3 ambient, vec3 diffuse, vec3 specular, float shininess) {

	// Ambient
	vec3 final_ambient = lightAmbient * ambient;

	// Diffuse
	vec3 normal;
	if (!u_RenderNormals || u_Solid) {
		normal = normalize(Normal);
	} else {
		normal = texture(material.normal, TexCoord).rgb * 2.0 - 1.0 ;
		normal = TBN * normalize(normal); 
	}

	float diff = max(dot(normal, lightDir),  0.0);
	vec3 final_diffuse = diff * diffuse * lightDiffuse;

	// Specular
	vec3 viewDir = normalize(CameraPos - FragPos);
	vec3 halfway = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfway), 0.0), shininess);
	vec3 final_specular = spec * specular * lightSpecular;

	vec3 result = final_ambient + final_diffuse + final_specular;
	return result;

}

vec3 CalculateDirLight(DirLight light, vec3 ambient, vec3 diffuse, vec3 specular, float shininess) {

	vec3 lightDir = normalize(-light.direction);
	vec3 spectrum = BlinnPhong(lightDir, light.ambient, light.diffuse, light.specular, ambient, diffuse, specular, shininess);

	return spectrum;
}

vec3 CalculatePointLight(PointLight light, vec3 ambient, vec3 diffuse, vec3 specular, float shininess) {
	vec3 lightDir = normalize(light.position - FragPos);
	vec3 spectrum = BlinnPhong(lightDir, light.ambient, light.diffuse, light.specular, ambient, diffuse, specular, shininess);

	float dist = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	vec3 result = attenuation * spectrum;
	return result;
}

vec3 CalculateSpotlight(Spotlight light, vec3 ambient, vec3 diffuse, vec3 specular, float shininess) {
	vec3 lightDir = normalize(light.position - FragPos);
	vec3 spectrum = BlinnPhong(lightDir, light.ambient, light.diffuse, light.specular, ambient, diffuse, specular, shininess);

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
	vec3 result = attenuation * intensity * spectrum;
	return result;
}

void main()
{
	TBN = mat3(normalize(T), normalize(B), normalize(Normal));
	vec3 result = vec3(0.0);
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;

	if (u_Solid) {
		ambient = u_SolidMaterial.ambient;
		diffuse = u_SolidMaterial.diffuse;
		specular = u_SolidMaterial.specular;
		shininess = u_SolidMaterial.shininess;
	} else {
		ambient = texture(material.ambient, TexCoord).rgb;
		diffuse = texture(material.diffuse, TexCoord).rgb;
		specular = texture(material.specular, TexCoord).rgb;
		shininess = material.shininess;
	}

	for (int i = 0; i < u_NrDirLights; i++) {
		result += CalculateDirLight(u_DirLights[i], ambient, diffuse, specular, shininess);
	}
	for (int i = 0; i < u_NrPointLights; i++) {
		result += CalculatePointLight(u_PointLights[i], ambient, diffuse, specular, shininess);
	}
	for (int i = 0; i < u_NrSpotlights; i++) {
		result += CalculateSpotlight(u_Spotlights[i], ambient, diffuse, specular, shininess);
	}

	if (u_Reinhard) {
		result = result / (result + vec3(1.0));
	}

	FragColor = vec4(result, 1.0);
}
