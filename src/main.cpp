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

#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

std::vector<Vertex> vertices = {
    // positions          // texture coords
    {{ 0.5f,  0.5f, 0.0f},   {1.0f, 1.0f}}, // top right
    {{ 0.5f, -0.5f, 0.0f},   {1.0f, 0.0f}}, // bottom right
    {{-0.5f, -0.5f, 0.0f},   {0.0f, 0.0f}}, // bottom left
    {{-0.5f,  0.5f, 0.0f},   {0.0f, 1.0f}}  // top left 
};
std::vector<uint32_t> indices = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

int main()
{
    Window window(SCR_WIDTH, SCR_HEIGHT);

    VertexBuffer vertex_buffer;
    Texture container_tex(std::string("resources/textures/container.jpg"), 0, false);
    Texture smiley_tex(std::string("resources/textures/awesomeface.png"), 1, true);
    ShaderProgram shader_prog("src/shaders/vert.glsl", "src/shaders/frag.glsl");

    Model model(
        &vertex_buffer, 
        vertices, 
        indices, 
        shader_prog, 
        {{"container", container_tex}, {"smiley", smiley_tex}}, 
        glm::mat4(1.0f));
    vertex_buffer.buffer_data();

    while (!window.should_close())
    {
        window.process_input();
        window.clear();

        model.set_mvp(glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.5f, 0.5f, 0.0f)));
        model.draw();

        window.swap_buffers();
        window.poll_events();
    }
    return 0;
}

