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

//glm::vec3 cubePositions[] = {
//  glm::vec3( 0.0f,  0.0f,  0.0f), 
//  glm::vec3( 2.0f,  5.0f, -15.0f), 
//  glm::vec3(-1.5f, -2.2f, -2.5f),  
//  glm::vec3(-3.8f, -2.0f, -12.3f),  
//  glm::vec3( 2.4f, -0.4f, -3.5f),  
//  glm::vec3(-1.7f,  3.0f, -7.5f),  
//  glm::vec3( 1.3f, -2.0f, -2.5f),  
//  glm::vec3( 1.5f,  2.0f, -2.5f), 
//  glm::vec3( 1.5f,  0.2f, -1.5f), 
//  glm::vec3(-1.3f,  1.0f, -1.5f)  
//};

std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f,  0.5f},  {0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f},  {1.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f},  {1.0f, 1.0f}},
    {{-0.5f,  0.5f,  0.5f},  {0.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f},  {0.0f, 1.0f}},
    {{ 0.5f, -0.5f, -0.5f},  {1.0f, 1.0f}},
    {{ 0.5f,  0.5f, -0.5f},  {1.0f, 0.0f}},
    {{-0.5f,  0.5f, -0.5f},  {0.0f, 0.0f}},
};
std::vector<uint32_t> indices = {
    0, 1, 2,
	2, 3, 0,
	1, 5, 6,
	6, 2, 1,
	7, 6, 5,
	5, 4, 7,
	4, 0, 3,
	3, 7, 4,
	4, 5, 1,
	1, 0, 4,
	3, 2, 6,
	6, 7, 3
};

int main()
{
    Camera camera(0.0f, 0.0f, 3.0f);
    Window window(SCR_WIDTH, SCR_HEIGHT, &camera);

    VertexBuffer vertex_buffer;
    //Texture container_tex(std::string("resources/textures/container.jpg"), 0, false);
    //Texture smiley_tex(std::string("resources/textures/awesomeface.png"), 1, true);
    ShaderProgram shader_prog({TRANSFORM, OBJECT_COLOR, LIGHT_COLOR}, "src/shaders/vert.glsl", "src/shaders/frag.glsl");
    ShaderProgram light_prog({TRANSFORM}, "src/shaders/vert.glsl", "src/shaders/light.glsl");

    Model cube(
        &vertex_buffer, 
        vertices, 
        indices, 
        shader_prog
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
        light_prog
    );
    light.model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 4.0f, -10.0f));

    vertex_buffer.buffer_data();

    glEnable(GL_DEPTH_TEST);
    while (!window.should_close())
    {
        window.process_input();
        window.clear();

        glm::mat4 view = camera.view();
        glm::mat4 projection = glm::perspective(glm::radians(50.0f), window.get_aspect_ratio(), 0.1f, 100.0f);
        glm::mat4 vp = projection * view;

        cube.mvp = vp * cube.model;
        cube.draw();

        light.mvp = vp * light.model;
        light.draw();

        window.swap_buffers();
        window.poll_events();
    }
    return 0;
}

