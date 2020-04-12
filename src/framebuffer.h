#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"

struct Framebuffer {

    Framebuffer(GLFWwindow *window);
    ~Framebuffer();

    void add_color_attachment();
    void add_depth_stencil_attachment();

    void bind();
    void draw();
    void recreate(int width, int height);

    static void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

protected:
    uint32_t id;
    uint32_t num_color_attachments = 0;

    uint32_t tex, rbo;

    ShaderProgram fb_shader;

    int width, height;

    uint32_t quad_vao, quad_vbo;

    void destroy_attachments();

    friend struct MultisampleFramebuffer;
};

struct MultisampleFramebuffer : public Framebuffer {
    
    MultisampleFramebuffer(GLFWwindow *window, int samples): Framebuffer(window), samples(samples) {}
    ~MultisampleFramebuffer() {
    }

    void resolve_to_framebuffer(Framebuffer &fb);
    void add_color_attachment();
    void add_depth_stencil_attachment();

private:
    uint32_t samples;
};