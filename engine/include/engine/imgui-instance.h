#pragma once

#include <GLFW/glfw3.h>
#include <stdbool.h>
#undef  IMGUI_IMPL_OPENGL_LOADER_GLEW
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui.h"
#include "engine/physics.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glm/glm.hpp"


struct ImGuiInstance {
    static bool gui_enabled, render_normals, render_skybox;
    static bool cull_back_face;
    static bool physics_enabled;
    static bool mask_overlay, fluid_overlay, fsdebug_scalar;
    static bool msaa, reinhard_hdr, wireframe;
    static bool draw_model_bb, draw_mesh_bb;
    static float camera_speed, camera_sensitivity, camera_fov;
    static float clear_r, clear_g, clear_b;
    static glm::vec3 *camera_pos;

    static ImGuiInstance *instance;

    ImGuiInstance(GLFWwindow *window, glm::vec3 *cpos);
    ~ImGuiInstance();
    void draw();

    static bool mouse_over_imgui();
};


