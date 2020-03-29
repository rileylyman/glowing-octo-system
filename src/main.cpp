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

glm::vec3 cubePositions[] = {
  glm::vec3( 0.0f,  0.0f,  0.0f), 
  glm::vec3( 2.0f,  5.0f, -15.0f), 
  glm::vec3(-1.5f, -2.2f, -2.5f),  
  glm::vec3(-3.8f, -2.0f, -12.3f),  
  glm::vec3( 2.4f, -0.4f, -3.5f),  
  glm::vec3(-1.7f,  3.0f, -7.5f),  
  glm::vec3( 1.3f, -2.0f, -2.5f),  
  glm::vec3( 1.5f,  2.0f, -2.5f), 
  glm::vec3( 1.5f,  0.2f, -1.5f), 
  glm::vec3(-1.3f,  1.0f, -1.5f)  
};

std::vector<Vertex> vertices = {
    // positions          // texture coords
    //{{ 0.5f,  0.5f, 0.0f},   {1.0f, 1.0f}}, // top right
    //{{ 0.5f, -0.5f, 0.0f},   {1.0f, 0.0f}}, // bottom right
    //{{-0.5f, -0.5f, 0.0f},   {0.0f, 0.0f}}, // bottom left
    //{{-0.5f,  0.5f, 0.0f},   {0.0f, 1.0f}}  // top left 
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
    //0, 1, 3, // first triangle
    //1, 2, 3  // second triangle
    0, 1, 2,
	2, 3, 0,
	// right
	1, 5, 6,
	6, 2, 1,
	// back
	7, 6, 5,
	5, 4, 7,
	// left
	4, 0, 3,
	3, 7, 4,
	// bottom
	4, 5, 1,
	1, 0, 4,
	// top
	3, 2, 6,
	6, 7, 3
};

int main()
{
    Camera camera(0.0f, 0.0f, 3.0f);
    Window window(SCR_WIDTH, SCR_HEIGHT, &camera);

    VertexBuffer vertex_buffer;
    Texture container_tex(std::string("resources/textures/container.jpg"), 0, false);
    Texture smiley_tex(std::string("resources/textures/awesomeface.png"), 1, true);
    ShaderProgram shader_prog("src/shaders/vert.glsl", "src/shaders/frag.glsl");

    Model mesh(
        &vertex_buffer, 
        vertices, 
        indices, 
        shader_prog, 
        {{"container", container_tex}, {"smiley", smiley_tex}}, 
        glm::mat4(1.0f));
    vertex_buffer.buffer_data();

    glEnable(GL_DEPTH_TEST);
    while (!window.should_close())
    {
        window.process_input();
        window.clear();

        for (int i = 0; i < 10; i++) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePositions[i]);
            model = glm::rotate(model, (float)glm::radians(glfwGetTime() * -55.0f), glm::vec3(1.0f, 0.5f, 0.0f)); 
            //glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
            glm::mat4 view = camera.view();
            glm::mat4 projection = glm::perspective(glm::radians(50.0f), window.get_aspect_ratio(), 0.1f, 100.0f);
            glm::mat4 mvp = (projection * view) * model;

            mesh.set_mvp(mvp);
            mesh.draw();
        }

        window.swap_buffers();
        window.poll_events();
    }
    return 0;
}

