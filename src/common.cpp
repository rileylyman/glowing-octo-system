#include "common.h"

float previous_delta_time = 0.0f;
float delta_time() {
    float current_time = glfwGetTime();
    float delta_time = current_time - previous_delta_time;
    previous_delta_time = current_time;
    return delta_time;
}

void draw_ray(glm::vec3 start, glm::vec3 end, glm::mat4 projection, glm::mat4 view) {
    glm::mat4 mat = projection * view;
    start = mat * glm::vec4(start, 1.0);
    end = mat * glm::vec4(end, 1.0);
    glBegin(GL_LINES);

    glVertex3f(start.x, start.y, start.z);
    glVertex3f(end.x, end.y, end.z);

    glEnd();
}