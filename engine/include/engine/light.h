#pragma once
#include <glm/glm.hpp>

struct DirLight {
	glm::vec3 direction = glm::vec3(-0.2, -1.0f, -0.3f);

	glm::vec3 ambient = glm::vec3(0.05f, 0.05f, 0.05f);
	glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
};

struct PointLight {
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 ambient = glm::vec3(0.05, 0.05, 0.05);
	glm::vec3 diffuse = glm::vec3(0.97f, 0.76f, 0.46f);
	glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

	float constant = 1.0f;
	float linear = 0.045f;
	float quadratic = 0.0075f;
};

struct Spotlight {
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 direction = glm::vec3(-0.2, -1.0f, -0.3f);

	glm::vec3 ambient = glm::vec3(0.05, 0.05, 0.05);
	glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

    float constant = 1.0f;
	float linear = 0.045f;
	float quadratic = 0.0075f;

	float cosPhi = glm::cos(glm::radians(12.5f));
	float cosGamma = glm::cos(glm::radians(17.0f));
};
