#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "shader.h"
#include "window.h"
#include "texture.h"
#include "vertex.h"
#include "model.h"
#include "camera.h"

#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

std::vector<Vertex> vertices = {
    //front face
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // bottom left
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // bottom right
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // top right
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // top left

    //right face
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // bottom left
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // top right
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // top left

    //left face
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // bottom left
    {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
    {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // top right
    {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // top left

    //bottom face
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // bottom left
    {{ 0.5f, -0.5f,  -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, // top right
    {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // top left

    //top face
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // bottom left
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // top right
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // top left

    //back face
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}, // bottom left
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}, // bottom right
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}}, // top right
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}}, // top left
};
std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20
};

int main()
{
    Camera camera(0.0f, 0.0f, 3.0f);
    Window window(SCR_WIDTH, SCR_HEIGHT, &camera);

    VertexBuffer vertex_buffer;
    //Texture container_tex(std::string("resources/textures/container.jpg"), 0, false);
    //Texture smiley_tex(std::string("resources/textures/awesomeface.png"), 1, true);
    ShaderProgram shader_prog({VIEW, NORMAL_MATRIX_VIEW, MODEL_VIEW, TRANSFORM, OBJECT_COLOR, LIGHT_COLOR}, "src/shaders/vert.glsl", "src/shaders/frag.glsl");
    ShaderProgram light_prog( {VIEW, NORMAL_MATRIX_VIEW, MODEL_VIEW, TRANSFORM}, "src/shaders/vert.glsl", "src/shaders/light.glsl");

    Model cube(
        &vertex_buffer, 
        vertices, 
        indices, 
        shader_prog,
        &camera
    );
    cube.model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -2.0f, -5.0f));
    cube.object_color = glm::vec3(1.0f, 0.5f, 0.31f);
    cube.light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    //cube.container = &container_tex;
    //cube.smiley = &smiley_tex;

    Model light(
        &vertex_buffer,
        vertices,
        indices,
        light_prog,
        &camera
    );
    light.model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 4.0f, -10.0f));

    vertex_buffer.buffer_data();

    glEnable(GL_DEPTH_TEST);
    while (!window.should_close())
    {
        window.process_input();
        window.clear();
    
        cube.shader_prog.use();
        glm::vec3 light_pos = glm::vec3(5.0f * cos(glfwGetTime() * 0.55) - 3.0f, 0.0f, 5.0f * sin(glfwGetTime() * 0.55) - 5.0f);
        shader_prog.setVec3("lightPos", light_pos);

        glm::mat4 view = camera.view();
        glm::mat4 projection = glm::perspective(glm::radians(50.0f), window.get_aspect_ratio(), 0.1f, 100.0f);
        glm::mat4 vp = projection * view;

        cube.mvp = vp * cube.model;
        cube.draw();


        light.mvp = vp * glm::translate(glm::mat4(1.0f), light_pos);
        light.draw();

        window.swap_buffers();
        window.poll_events();
    }
    return 0;
}

