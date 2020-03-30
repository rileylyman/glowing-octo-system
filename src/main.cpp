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
#include "common.h"

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
    Texture container_tex(std::string("resources/textures/container2.png"), 0, true);
    Texture container_tex_spec(std::string("resources/textures/container2_specular.png"), 1, true);
    ShaderProgram shader_prog({VIEW, NORMAL_MATRIX_VIEW, MODEL_VIEW, TRANSFORM, MATERIAL}, "src/shaders/vert.glsl", "src/shaders/frag.glsl");
    ShaderProgram light_prog( {VIEW, NORMAL_MATRIX_VIEW, MODEL_VIEW, TRANSFORM}, "src/shaders/vert.glsl", "src/shaders/light.glsl");

    Model cube(
        &vertex_buffer, 
        vertices, 
        indices, 
        shader_prog,
        &camera
    );
    cube.model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -2.0f, -5.0f));
    cube.material.diffuse = &container_tex;
    cube.material.specular = &container_tex_spec;
    cube.material.shininess = 16.0f;

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
        glm::vec3 light_pos = glm::vec3(-3.0f, 2.0f, -5.0f);
        shader_prog.setVec3("lightU.position", light_pos);
        shader_prog.setVec3("lightU.ambient", 0.2f, 0.2f, 0.2f);
        shader_prog.setVec3("lightU.diffuse", 0.5f, 0.5f, 0.5f);
        shader_prog.setVec3("lightU.specular", 1.0f, 1.0f, 1.0f);


        cube.model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -2.0f, -5.0f));
        cube.model = glm::rotate(cube.model, (float)glfwGetTime() * 0.7f, glm::vec3(1.0f, 0.0f, 0.0f));
        cube.draw();

        light.shader_prog.use();
        light.model = glm::translate(glm::mat4(1.0f), light_pos);
        light.shader_prog.setVec3("lightColor", 1.0f, 1.0f, 1.0f); 
    
        light.draw();

        window.swap_buffers();
        window.poll_events();
    }
    return 0;
}

