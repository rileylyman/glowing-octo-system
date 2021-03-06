#pragma once 

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "engine/common.h"

struct Camera {
    float yaw = -glm::half_pi<float>();
    float pitch = 0.0f;
    float aspect_ratio = 800.0f / 600.0f;
    glm::vec3 position = {0.0, 0.0, 0.0}, front = {0.0, 0.0, 1.0}, right = {1.0, 0.0, 0.0}, up = {0.0, 1.0, 0.0};
    const glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    const float pitch_limit = glm::half_pi<float>() - 0.1f;


    bool first_mouse = true;
    double last_x, last_y;

    Camera(float x, float y, float z);
    void keyboard_input(GLFWwindow *window);
    void mouse_input(double xpos, double ypos);
    glm::mat4 view();
    glm::mat4 projection();
};