#include <glad/glad.h>
#include <stdlib.h>
#include <iostream>
#include "log.h"
#include "window.h"
#include "imgui-instance.h"

Window::Window(uint32_t width, uint32_t height, Camera *cam) : cam(cam), width(width), height(height) {

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
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowFocusCallback(window, focus_callback);
    glfwSetWindowUserPointer(window, this);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    mouse_locked = false;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }

    set_clear_color(0.6f, 0.5f, 0.6f, 1.0f);
}

Window::~Window() { 
    glfwTerminate();
}

void Window::poll_events() {
    glfwPollEvents();
}

void Window::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::set_clear_color(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void Window::swap_buffers() {
    glfwSwapBuffers(window);
}

bool Window::should_close() {
    return glfwWindowShouldClose(window);
}

float Window::get_aspect_ratio() {
    return (float)width/(float)height;
}

void Window::process_input() {
    cam->keyboard_input(window);
}

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
    Window *user_window = (Window *)glfwGetWindowUserPointer(window);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && user_window->mouse_locked) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        user_window->mouse_locked = false;
    } else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        //glfwSetWindowShouldClose(window, true);
    } else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        user_window->mouse_locked = true;
    } else if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS)  {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        user_window->mouse_locked = false;
        ImGuiInstance::gui_enabled = !ImGuiInstance::gui_enabled;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Window *user_window = (Window *)glfwGetWindowUserPointer(window);
    user_window->cam->aspect_ratio = (float)width / (float)height;
    user_window->width = width;
    user_window->height = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    Window *user_window = (Window *)glfwGetWindowUserPointer(window);
    if (user_window->mouse_locked) {
        user_window->cam->mouse_input(xpos, ypos);
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (ImGuiInstance::mouse_over_imgui()) {
            std::cout << "Clicked imgui" << std::endl;
        } else {
            std::cout << "Click" << std::endl;
        }
    }
}

void focus_callback(GLFWwindow *window, int focus) {
    if (!focus) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        Window *current_window = (Window *)glfwGetWindowUserPointer(window);
        current_window->mouse_locked = false;
    }
}