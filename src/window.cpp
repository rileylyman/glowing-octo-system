#include <glad/glad.h>
#include <stdlib.h>
#include <iostream>
#include "log.h"
#include "window.h"

Window::Window(uint32_t width, uint32_t height) : width(width), height(height) {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
#endif

    window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);
    if (window == nullptr) {
        std::cout << "Failed to create window\n";
        //log_fatal("Failed to create window!\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }

    set_clear_color(0.2f, 0.5f, 0.3f, 1.0f);
}

Window::~Window() { 
    glfwTerminate();
}

void Window::poll_events() {
    glfwPollEvents();
}

void Window::clear() {
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::set_clear_color(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void Window::swap_buffers() {
    glfwSwapBuffers(window);
}

void Window::process_input() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

bool Window::should_close() {
    return glfwWindowShouldClose(window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}