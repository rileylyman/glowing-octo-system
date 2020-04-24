#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

float delta_time(); 

void draw_ray(glm::vec3 start, glm::vec3 end, glm::mat4 projection, glm::mat4 view); 