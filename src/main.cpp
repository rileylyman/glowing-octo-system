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
#include "framebuffer.h"

#include <iostream>

Model construct_sphere(VertexBuffer *, float, int, int, MeshShaderType);

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
    ShaderProgram shader_prog("src/shaders/vert.glsl", "src/shaders/pbr.frag");
    //ShaderProgram shader_prog("src/shaders/vert.glsl", "src/shaders/frag.glsl");
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

    Framebuffer fb(window.window);
    fb.add_color_attachment();
    fb.add_depth_stencil_attachment();

    MultisampleFramebuffer msfb(window.window, 16);
    msfb.add_color_attachment();
    msfb.add_depth_stencil_attachment();

    window.framebuffers_to_alert.push_back(&fb); // Alert this framebuffer on window resize
    window.framebuffers_to_alert.push_back(&msfb); 

    //
    // Create buffer which will hold model vertex data 
    //
    VertexBuffer vertex_buffer;

    //
    // Load models
    //
    //Model rifle(&vertex_buffer, "resources/models/suitofnano/nanosuit.obj", BP_TEXTURED, 0, true);
    Model rifle(&vertex_buffer, "resources/models/super/scene.fbx", PBR_TEXTURED, 0, false);
    //Model pbrpistol(&vertex_buffer, "resources/models/pbrpistol/scene.fbx", PBR_TEXTURED, 0, false);

    Model sphere = construct_sphere(&vertex_buffer, 2.0f, 64, 64, PBR_SOLID);

    vertex_buffer.buffer_data();

    //
    // Set up lights
    //
    DirLight dir_light0;
    dir_light0.direction = glm::vec3(-1.0f, -1.0f, -1.0f);
    dir_light0.diffuse = glm::vec3(200.0f, 200.0f, 200.0f);
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

    std::vector<DirLight*> dir_lights = { &dir_light0, &dir_light1, &dir_light2, &dir_light3, &dir_light4, &dir_light5, &dir_light6, &dir_light7 };
    //std::vector<DirLight *> dir_lights = { &dir_light0 };
    std::vector<PointLight*> point_lights = { &point_light0, &point_light1, &point_light2, &point_light3 };
    std::vector<Spotlight*> spotlights = { &spot_light };
    //std::vector<DirLight*> dir_lights = {};
    //std::vector<PointLight*> point_lights = {};

    shader_prog.bind_lights(dir_lights, point_lights, spotlights);

    //
    // Render loop
    //
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_MULTISAMPLE);
    //glfwSwapInterval(0);
    while (!window.should_close())
    {
        if (ImGuiInstance::msaa) {
            msfb.bind();
        } else {
            fb.bind();
        }

        window.process_input();
        window.set_clear_color(ImGuiInstance::clear_r, ImGuiInstance::clear_g, ImGuiInstance::clear_b, 1.0f);
        window.clear();
    
        shader_prog.use();
        spot_light.position = camera.position;
        spot_light.direction = camera.front;
        shader_prog.bind_lights(dir_lights, point_lights, spotlights);

        if (ImGuiInstance::reinhard_hdr) {
            shader_prog.setBool("u_Reinhard", true);
        } else {
            shader_prog.setBool("u_Reinhard", false);
        }
        //nanosuit.model = glm::rotate(glm::mat4(1.0f), (float) glfwGetTime() * 0.02f, glm::vec3(0.0, 1.0, 0.0));
        rifle.draw(shader_prog, &camera);
        if (ImGuiInstance::draw_model_bb) rifle.draw_bounding_box(&camera);

        sphere.model = glm::translate(glm::mat4(1.0f), glm::vec3(glfwGetTime(), 0.0f, 0.0f));
        sphere.draw(shader_prog, &camera);

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

        if (ImGuiInstance::msaa) {
            msfb.resolve_to_framebuffer(fb);
        }
        Framebuffer::unbind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        fb.draw();

        window.swap_buffers();
        window.poll_events();
    }
    return 0;
}

Model construct_sphere(VertexBuffer *vertex_buffer, float radius, int sectorCount, int stackCount, MeshShaderType shader_type) {

    std::vector<Vertex> vertices;

    const float PI = 3.14159265359;
    float x, y, z, xy;                           // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius; // vertex normal
    float s, t;                                  // vertex texCoord

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);      // r * cos(u)
        z = radius * sinf(stackAngle);       // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j)
        {
            Vertex v;
            sectorAngle = j * sectorStep; // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
            v.position = glm::vec3(x,y,z);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            v.normal = glm::vec3(nx,ny,nz);
            v.tangent = glm::vec3(nx,ny,nz);
            v.bitangent = glm::vec3(nx,ny,nz);

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            v.tex_coord = glm::vec2(s,t);

            vertices.push_back(v);
        }
    }

    // generate CCW index list of sphere triangles
    std::vector<uint32_t> indices;
    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1); // beginning of current stack
        k2 = k1 + sectorCount + 1;  // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    if (shader_type == BP_SOLID) {
        return Model(vertex_buffer, vertices, indices, BlinnPhongSolidMaterial{ glm::vec3(0.3f), glm::vec3(0.6f), glm::vec3(1.0f), 64.0f});
    } else if (shader_type == PBR_SOLID) {
        return Model(vertex_buffer, vertices, indices, PBRSolidMaterial{ glm::vec3(0.7f, 0.1f, 0.1f), 1.0f, 0.3f });
    } else {
        throw "Sphere with this shader type is not yet supported";
    }
}