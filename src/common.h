#include <GLFW/glfw3.h>

static float previous_delta_time = 0.0f;
float delta_time() {
    float current_time = glfwGetTime();
    float delta_time = current_time - previous_delta_time;
    previous_delta_time = current_time;
    return delta_time;
}