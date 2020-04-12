#include "framebuffer.h"
#include <iostream>

Framebuffer::Framebuffer(GLFWwindow *window): fb_shader("src/shaders/fb.vert", "src/shaders/fb.frag") {
    glGenFramebuffers(1, &id);
    glfwGetWindowSize(window, &width, &height);

    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);

    float data[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_VERTEX_ARRAY, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::recreate(int new_width, int new_height) {
    width = new_width;
    height = new_height;

    //
    // TODO: For more general solution, keep a list of
    // attachment infos, so that we can always recreate what
    // the user wanted
    //
    destroy_attachments();
    add_color_attachment();
    add_depth_stencil_attachment();
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &id);
}

void Framebuffer::add_color_attachment() {
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + num_color_attachments++, GL_TEXTURE_2D, tex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MultisampleFramebuffer::add_color_attachment() {
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);

    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_SRGB, width, height, GL_TRUE);

    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + num_color_attachments++, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Framebuffer::add_depth_stencil_attachment() {
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MultisampleFramebuffer::add_depth_stencil_attachment() {
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroy_attachments() {
    glDeleteTextures(1, &tex);
    glDeleteRenderbuffers(1, &rbo);
    num_color_attachments = 0;
    tex = 0;
    rbo = 0;
}

void Framebuffer::bind() {
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw "cannot bind incomplete framebuffer!";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void Framebuffer::draw() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    fb_shader.use();
    fb_shader.setInt("u_ScreenTexture", 0);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void MultisampleFramebuffer::resolve_to_framebuffer(Framebuffer &fb) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.id);
    glBlitFramebuffer(0, 0, width, height, 0, 0, fb.width, fb.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
