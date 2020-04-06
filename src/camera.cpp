#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui-instance.h"
#include "camera.h"
#include "common.h"

Camera::Camera(float x, float y, float z) : position(glm::vec3(x, y, z)), last_x(400), last_y(300) {}

void Camera::keyboard_input(GLFWwindow *window) {
    float velocity = ImGuiInstance::camera_speed * delta_time();
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        position += velocity * front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        position -= velocity * front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        position -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        position += right * velocity;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        position += up * velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        position -= up * velocity;
}

void Camera::mouse_input(double xpos, double ypos) {

    if (first_mouse) {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false; 
    }

    float xoff = (xpos - last_x) * ImGuiInstance::camera_sensitivity;
    float yoff = (ypos - last_y) * ImGuiInstance::camera_sensitivity;
    last_x = xpos;
    last_y = ypos;

    yaw += xoff;
    pitch -= yoff;
    if (pitch > pitch_limit) {
        pitch = pitch_limit;
    } else if (pitch < -pitch_limit) {
        pitch = -pitch_limit;
    }

    glm::vec3 direction;
    direction.x = cos(yaw) * cos(pitch);
    direction.y = sin(pitch);
    direction.z = sin(yaw) * cos(pitch);
    front = glm::normalize(direction);
    right = glm::normalize(glm::cross(front, world_up));
    up = glm::normalize(glm::cross(right, front));
}

glm::mat4 Camera::view() {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::projection() {
    return glm::perspective(glm::radians(ImGuiInstance::camera_fov), aspect_ratio, 0.1f, 1000.0f);
}