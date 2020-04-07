#pragma once

#include <GLFW/glfw3.h>
#include "camera.h"

struct Window {
    uint32_t width, height;
    GLFWwindow *window;
    Camera *cam;
    bool mouse_locked = true;

    Window(uint32_t width, uint32_t height, Camera *cam);
    ~Window();
    bool should_close();
    void swap_buffers();
    void poll_events();
    void clear();
    void set_clear_color(float r, float g, float b, float a);
    float get_aspect_ratio();
    void process_input();
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height); 
void mouse_callback(GLFWwindow *window, double xpos, double ypos); 
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods); 
void focus_callback(GLFWwindow *window, int focus);
