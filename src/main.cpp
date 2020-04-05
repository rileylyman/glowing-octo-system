#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui-instance.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "shader.h"
#include "skybox.h"
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
    gladLoadGL();
    //
    // Set up window
    //
    Camera camera(0.0f, 0.0f, 3.0f);
    Window window(SCR_WIDTH, SCR_HEIGHT, &camera);

    //
    // Set up imgui instance
    //
    ImGuiInstance imgui_instance(window.window);

    //
    // Load assets
    //
    ShaderProgram shader_prog({CAMERA_POS, NORMAL_MATRIX, MODEL, TRANSFORM, MATERIAL}, "src/shaders/vert.glsl", "src/shaders/frag.glsl");
    ShaderProgram light_prog( {CAMERA_POS, NORMAL_MATRIX, MODEL, TRANSFORM}, "src/shaders/vert.glsl", "src/shaders/light.glsl");

    //
    // Get skybox cubemap texture
    //
    Skybox skybox(
        {
            "resources/textures/skybox/right.jpg",
            "resources/textures/skybox/left.jpg",
            "resources/textures/skybox/top.jpg",
            "resources/textures/skybox/bottom.jpg",
            "resources/textures/skybox/front.jpg",
            "resources/textures/skybox/back.jpg",
        },
        "src/shaders/skybox.vert",
        "src/shaders/skybox.frag"
    );

    //
    // Buffer vertex data and set materials
    //
    VertexBuffer vertex_buffer;
    Model nanosuit(&vertex_buffer, "resources/models/suitofnano/nanosuit.obj", true);
    nanosuit.model = glm::scale(glm::mat4(1.0f), glm::vec3(.01f));

    //Model head(&vertex_buffer, "resources/models/head/scene.obj", true);

    Model rifle(&vertex_buffer, "resources/models/rifle/scene.gltf", false);
    rifle.model = glm::scale(glm::mat4(1.0f), glm::vec3(.01f));
    vertex_buffer.buffer_data();

    //
    // Set up lights
    //
    DirLight dir_light;
    dir_light.direction = glm::vec3(0.0f, -1.0f, -1.0f);
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
    //glfwSwapInterval(0);
    while (!window.should_close())
    {
        window.process_input();
        window.set_clear_color(ImGuiInstance::clear_r, ImGuiInstance::clear_g, ImGuiInstance::clear_b, 1.0f);
        window.clear();

    
        shader_prog.use();
        spot_light.position = camera.position;
        spot_light.direction = camera.front;
        shader_prog.bind_lights(camera.view(), dir_lights, point_lights, spotlights);

        nanosuit.draw(shader_prog, &camera);
        //head.draw(shader_prog, &camera);
        //rifle.draw(shader_prog, &camera);

        //light_prog.use();
        //for (int i = 0; i < 4; i++) {
        //    //light.model = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)), light_positions[i]);
        //    //light_prog.setVec3("lightColor", 1.0f, 1.0f, 1.0f); 
        //    //light.draw(light_prog, &camera);
        //}

        if (ImGuiInstance::render_skybox)
            skybox.draw(&camera);

        imgui_instance.draw();

        window.swap_buffers();
        window.poll_events();
    }
    return 0;
}

