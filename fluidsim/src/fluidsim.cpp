#include <iostream>
#include "fluidsim/fluidsim.h"

#include <engine/texture.h>
#include <engine/framebuffer.h>
#include <engine/shader.h>
#include <glad/glad.h>

void fluidsim_testing123() {

    Texture3D grid0(100, 100, 100, 0);
    Texture3D grid1(100, 100, 100, 1);

    Framebuffer framebuffer(100, 100);
    framebuffer.add_color_attachment();
    framebuffer.add_depth_stencil_attachment();
    framebuffer.bind();

    uint32_t cube_vao, cube_vbo;
    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);

    float data[] = {    
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    ShaderProgram shader("src/shaders/fluid.vert", "src/shaders/fluid.frag");
    shader.use();

    grid0.use();
    shader.setInt("u_Grid0", grid0.unit);
    
    grid1.use();
    shader.setInt("u_Grid1", grid1.unit);
    
    //shader.setFloat("u_Depth", 0.0f);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    framebuffer.unbind();

    uint32_t final_texture = framebuffer.get_color_texture();

    std::cout << "Hello from fluidsim!" << std::endl;
}
