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
#include "light.h"

#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 light_positions[] = {
    glm::vec3( 0.7f,  10.0f,  2.0f),
    glm::vec3( 12.3f, -3.3f, -4.0f),
    glm::vec3(-14.0f,  2.0f, -12.0f),
    glm::vec3( 0.0f,  10.0f, -3.0f)
};

int main()
{
    //
    // Set up window
    //
    Camera camera(0.0f, 0.0f, 3.0f);
    Window window(SCR_WIDTH, SCR_HEIGHT, &camera);

    //
    // Load assets
    //
    ShaderProgram shader_prog({CAMERA_POS, NORMAL_MATRIX, MODEL, TRANSFORM, MATERIAL}, "src/shaders/vert.glsl", "src/shaders/frag.glsl");
    ShaderProgram light_prog( {CAMERA_POS, NORMAL_MATRIX, MODEL, TRANSFORM}, "src/shaders/vert.glsl", "src/shaders/light.glsl");

    //
    // Buffer vertex data and set materials
    //
    VertexBuffer vertex_buffer;
    Model nanosuit(&vertex_buffer, "resources/models/rifle/scene.gltf");
    nanosuit.model = glm::scale(glm::mat4(1.0f), glm::vec3(.01f));


    vertex_buffer.buffer_data();

    //
    // Set up lights
    //
    DirLight dir_light;
    PointLight point_light0 = { .position = light_positions[0] };
    PointLight point_light1 = { .position = light_positions[1] };
    PointLight point_light2 = { .position = light_positions[2] };
    PointLight point_light3 = { .position = light_positions[3] };
    Spotlight  spot_light   = { .position = camera.position, .direction = camera.front };

    std::vector<DirLight*> dir_lights = { &dir_light };
    std::vector<PointLight*> point_lights = { &point_light0, &point_light1, &point_light2, &point_light3 };
    std::vector<Spotlight*> spotlights = { &spot_light };

    //
    // Render loop
    //
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    while (!window.should_close())
    {
        window.process_input();
        window.clear();
    
        shader_prog.use();
        spot_light.position = camera.position;
        spot_light.direction = camera.front;
        shader_prog.bind_lights(camera.view(), dir_lights, point_lights, spotlights);

        nanosuit.model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * 0.5f, glm::vec3(0.0f, 0.5f, 0.5f));
        nanosuit.draw(shader_prog, &camera);

        //light_prog.use();
        //for (int i = 0; i < 4; i++) {
        //    //light.model = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)), light_positions[i]);
        //    //light_prog.setVec3("lightColor", 1.0f, 1.0f, 1.0f); 
        //    //light.draw(light_prog, &camera);
        //}

        window.swap_buffers();
        window.poll_events();
    }
    return 0;
}

