#pragma once

#include <GLFW/glfw3.h>

struct Window {
    uint32_t width, height;
    GLFWwindow *window;

    Window(uint32_t width, uint32_t height);
    ~Window();
    void process_input();
    bool should_close();
    void swap_buffers();
    void poll_events();
    void clear();
    void set_clear_color(float r, float g, float b, float a);
};


void framebuffer_size_callback(GLFWwindow* window, int width, int height); 