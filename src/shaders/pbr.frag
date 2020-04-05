#version 330 core

out vec4 FragColor;
in vec2 TexCoord;
in mat3 TBN;
in vec3 Normal;
in vec3 FragPos;
in vec3 CameraPos;

struct Material {
    sampler2D albedo;
    sampler2D normal;
    sampler2D metallic;
    sampler2D roughness;
    sampler2D ao;
}
uniform Material u_Material;

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

const float PI = 3.14159265359;

//
// F0 - the Fresnel effect at 0-degree incidence
// For dielectrics, good approximation is F0=vec3(0.4)
// If a surface is metallic, the Fresnel effect actually has
// a tint and is much larger. Achieve by
// F0 = mix(vec3(0.4), surfaceColor.rgb, metalness)
//
float FresnelSchlick(vec3 H, vec3 V, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - max(dot(H, V), 0.0), 5.0);)
}

float GeometrySchlickGGX(float NdotV, float k) {
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, k) * GeometrySchlickGGX(NdotL, k);
}

float NDFTrowbridgeReitzGGX(vec3 N, vec3 H, float a) {
    float a2 = a * a;
    float NdotH = dot(N, H);
    float denom = NdotH * NdotH * (a2 - 1) + 1;
    return a2 / (PI * denom * denom);
}

void main() {
    vec3 albedo = texture(u_Material.albedo, TexCoord).rgb;
	vec3 normal = texture(u_Material.normal, TexCoord).rgb * 2.0 - 1.0 ;
	normal = TBN * normalize(normal); 
    float metallic = texture(u_Material.metallic, TexCoord).r;
    float roughness = texture(u_Material.roughness, TexCoord).r;
    float ao = texture(u_Material.ao, TexCoord).r;

    vec3 N = normalize(normal;
    vec3 V = normalize(CameraPos - FragPos);
    
    vec3 F0 = vec3(0.4);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < u_NrPointLights; i++) {
        PointLight light = u_PointLights[i];

        vec3 L = normalize(u_PointLights[i].position - FragPos);
        vec3 H = normalize(V + L);

        float distance = length(light.position - FragPos);
	    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
        vec3 radiance = light[i].color * attenuation;

        float NDF = NDFTrowbridgeReitzGGX(N, H, roughness);
        float k = roughness + 1; k = k * k / 8.0; 
        float G = GeometrySmith(N, V, L, k);
        vec3 F = FresnelSchlick(H, V, F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = numerator / max(denom, 0.001);

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));

    FragColor = vec4(color, 1.0);
}