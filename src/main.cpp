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

void renderSphere();

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 light_positions[] = {
    glm::vec3( 17.2,  18.2f,  9.3f),
    glm::vec3( 27.0f, 25.0f, -67.5f),
    glm::vec3(-28.4f, 23.4f, -37.2f),
    glm::vec3( 0.0f,  10.0f, -30.0f)
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
    ImGuiInstance imgui_instance(window.window, &camera.position);

    //
    // Load assets
    //
    //ShaderProgram shader_prog("src/shaders/vert.glsl", "src/shaders/pbr.frag");
    ShaderProgram shader_prog("src/shaders/vert.glsl", "src/shaders/frag.glsl");
    ShaderProgram light_prog("src/shaders/vert.glsl", "src/shaders/light.glsl");

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
    // Create buffer which will hold model vertex data 
    //
    VertexBuffer vertex_buffer;

    //
    // Load models
    //
    Model rifle(&vertex_buffer, "resources/models/suitofnano/nanosuit.obj", BP_TEXTURED, 0, true);
    //Model rifle(&vertex_buffer, "resources/models/super/scene.fbx", PBR_TEXTURED, 0, false);
    //Model pbrpistol(&vertex_buffer, "resources/models/pbrpistol/scene.fbx", PBR_TEXTURED, 0, false);

    vertex_buffer.buffer_data();

    //
    // Set up lights
    //
    DirLight dir_light0;
    dir_light0.direction = glm::vec3(-1.0f, -1.0f, -1.0f);
    DirLight dir_light1;
    dir_light1.direction = glm::vec3(-1.0f, -1.0f, 1.0f);
    DirLight dir_light2;
    dir_light2.direction = glm::vec3(-1.0f, 1.0f, -1.0f);
    DirLight dir_light3;
    dir_light3.direction = glm::vec3(-1.0f, 1.0f, 1.0f);
    DirLight dir_light4;
    dir_light4.direction = glm::vec3(1.0f, -1.0f, -1.0f);
    DirLight dir_light5;
    dir_light5.direction = glm::vec3(1.0f, -1.0f, 1.0f);
    DirLight dir_light6;
    dir_light6.direction = glm::vec3(1.0f, 1.0f, -1.0f);
    DirLight dir_light7;
    dir_light7.direction = glm::vec3(1.0f, 1.0f, 1.0f);
    PointLight point_light0 = { .position = light_positions[0] };
    PointLight point_light1 = { .position = light_positions[1] };
    PointLight point_light2 = { .position = light_positions[2] };
    PointLight point_light3 = { .position = light_positions[3] };
    Spotlight  spot_light   = { .position = camera.position, .direction = camera.front };

    //std::vector<DirLight*> dir_lights = { &dir_light0, &dir_light1, &dir_light2, &dir_light3, &dir_light4, &dir_light5, &dir_light6, &dir_light7 };
    std::vector<PointLight*> point_lights = { &point_light0, &point_light1, &point_light2, &point_light3 };
    std::vector<Spotlight*> spotlights = { &spot_light };
    std::vector<DirLight*> dir_lights = {};
    //std::vector<PointLight*> point_lights = {};

    shader_prog.bind_lights(dir_lights, point_lights, spotlights);

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
        shader_prog.bind_lights(dir_lights, point_lights, spotlights);

        //nanosuit.model = glm::rotate(glm::mat4(1.0f), (float) glfwGetTime() * 0.02f, glm::vec3(0.0, 1.0, 0.0));
        rifle.draw(shader_prog, &camera);
        if (ImGuiInstance::draw_model_bb) rifle.draw_bounding_box(&camera);

        //light_prog.use();
        //for (int i = 0; i < 4; i++) {
        //    //light.model = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)), light_positions[i]);
        //    //light_prog.setVec3("lightColor", 1.0f, 1.0f, 1.0f); 
        //    //light.draw(light_prog, &camera);
        //}

        draw_ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 100.0f, 0.0f), camera.projection(), camera.view());

        if (ImGuiInstance::render_skybox)
            skybox.draw(&camera);

        imgui_instance.draw();

        window.swap_buffers();
        window.poll_events();
    }
    return 0;
}

unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359;
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (unsigned int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = indices.size();

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            for (int _ = 0; _ < 6; _++) {
                data.push_back(0.0f);
            } 
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        float stride = (3 + 2 + 3 + 3 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(12 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

