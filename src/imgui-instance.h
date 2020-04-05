#pragma once

#include <GLFW/glfw3.h>
#include <stdbool.h>
#undef  IMGUI_IMPL_OPENGL_LOADER_GLEW
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


struct ImGuiInstance {
    static bool gui_enabled, render_normals, render_skybox;
    static bool cull_back_face;
    static float camera_speed, camera_sensitivity, camera_fov;
    static float clear_r, clear_g, clear_b;

    ImGuiInstance(GLFWwindow *window);
    ~ImGuiInstance();
    void draw();
};


