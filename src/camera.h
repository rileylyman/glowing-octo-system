#pragma once 

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "common.h"

struct Camera {
    float yaw = -glm::half_pi<float>();
    float pitch = 0.0f;
    float camera_speed = 0.1f, sensitivity = 0.010f;
    glm::vec3 position, front, right, up;
    const glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    const float pitch_limit = glm::half_pi<float>() - 0.1f;

    float fov = 50.0f;

    bool first_mouse = true;
    double last_x, last_y;

    Camera(float x, float y, float z);
    void keyboard_input(GLFWwindow *window);
    void mouse_input(double xpos, double ypos);
    glm::mat4 view();
    glm::mat4 projection();
};