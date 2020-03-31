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

//std::vector<Vertex> vertices = {
//    //front face
//    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // bottom left
//    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // bottom right
//    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // top right
//    {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // top left
//
//    //right face
//    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // bottom left
//    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
//    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // top right
//    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // top left
//
//    //left face
//    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // bottom left
//    {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
//    {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // top right
//    {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // top left
//
//    //bottom face
//    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // bottom left
//    {{ 0.5f, -0.5f,  -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
//    {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, // top right
//    {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // top left
//
//    //top face
//    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // bottom left
//    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
//    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // top right
//    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // top left
//
//    //back face
//    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}, // bottom left
//    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}, // bottom right
//    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}}, // top right
//    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}}, // top left
//};
//std::vector<uint32_t> indices = {
//    0, 1, 2, 2, 3, 0,
//    4, 5, 6, 6, 7, 4,
//    8, 9, 10, 10, 11, 8,
//    12, 13, 14, 14, 15, 12,
//    16, 17, 18, 18, 19, 16,
//    20, 21, 22, 22, 23, 20
//};

glm::vec3 light_positions[] = {
    glm::vec3( 0.7f,  10.0f,  2.0f),
    glm::vec3( 12.3f, -3.3f, -4.0f),
    glm::vec3(-14.0f,  2.0f, -12.0f),
    glm::vec3( 0.0f,  10.0f, -3.0f)
};

glm::vec3 cube_positions[] = {
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
    //Texture container_tex(std::string("resources/textures/container2.png"), 0);
    //Texture container_tex_spec(std::string("resources/textures/container2_specular.png"), 1);
    ShaderProgram shader_prog({CAMERA_POS, NORMAL_MATRIX, MODEL, TRANSFORM, MATERIAL}, "src/shaders/vert.glsl", "src/shaders/frag.glsl");
    ShaderProgram light_prog( {CAMERA_POS, NORMAL_MATRIX, MODEL, TRANSFORM}, "src/shaders/vert.glsl", "src/shaders/light.glsl");

    //
    // Buffer vertex data and set materials
    //
    VertexBuffer vertex_buffer;
    Model nanosuit(&vertex_buffer, "resources/models/torch/scene.gltf");
    nanosuit.model = glm::mat4(1.0f);//glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)), glm::vec3(0.0f, 0.0f, -3.0f));

    //Model cube(
    //    &vertex_buffer, 
    //    vertices, 
    //    indices, 
    //    shader_prog,
    //    &camera
    //);
    //cube.model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -2.0f, -5.0f));
    //cube.material.diffuse = &container_tex;
    //cube.material.specular = &container_tex_spec;
    //cube.material.shininess = 16.0f;

    //Model light(
    //    &vertex_buffer,
    //    vertices,
    //    indices,
    //    light_prog,
    //    &camera
    //);
    //light.model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 4.0f, -10.0f));

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
    glEnable(GL_DEPTH_TEST);
    while (!window.should_close())
    {
        window.process_input();
        window.clear();
    
        shader_prog.use();
        spot_light.position = camera.position;
        spot_light.direction = camera.front;
        shader_prog.bind_lights(camera.view(), dir_lights, point_lights, spotlights);


            //cube.model = glm::translate(glm::mat4(1.0f), cube_positions[i]);
            //cube.model = glm::rotate(cube.model, (float)glfwGetTime() * (float)i * 0.07f, glm::vec3(1.0f, 0.0f, 0.0f));
            //cube.draw(shader_prog, &camera);
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

