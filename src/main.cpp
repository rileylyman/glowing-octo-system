#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/imgui-instance.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "engine/shader.h"
#include "engine/fsrender.h"
#include "engine/skybox.h"
#include "engine/window.h"
#include "engine/texture.h"
#include "engine/vertex.h"
#include "engine/model.h"
#include "engine/camera.h"
#include "engine/common.h"
#include "engine/light.h"
#include "engine/scene.h"
#include "engine/framebuffer.h"

#include <fluidsim/fluidsim.h>

#include <iostream>

Model construct_sphere(VertexBuffer *, float, int, int, MeshShaderType);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
    gladLoadGL();
    //
    // Set up window
    //
    Camera camera(0.0f, 0.0f, 3.0f);
    Window window(SCR_WIDTH, SCR_HEIGHT, &camera);

    //
    // Set basic OpenGL settings. Must be done after creating the
    // window since it requires an OpenGL context
    //
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_MULTISAMPLE);
    //glfwSwapInterval(0);

    //
    // Set up imgui instance
    //
    ImGuiInstance imgui_instance(window.window, &camera.position);

    fluidsim_testing123();

    Framebuffer fb(window.window);
    fb.add_color_attachment();
    fb.add_depth_stencil_attachment();

    MultisampleFramebuffer msfb(window.window, 16);
    msfb.add_color_attachment();
    msfb.add_depth_stencil_attachment();

    window.framebuffers_to_alert.push_back(&fb); // Alert this framebuffer on window resize
    window.framebuffers_to_alert.push_back(&msfb); 

    //Model sphere = construct_sphere(&vertex_buffer, 2.0f, 64, 64, PBR_SOLID);

    VertexBuffer vertex_buffer;
    Scene scene("src/scenes/physics-test.json", &vertex_buffer);
    vertex_buffer.buffer_data();

    FluidDebugRenderer fsdebug(&camera, 10.0f, 5.0f, -10.0f);    
    uint32_t grid_width = 100, grid_height = 100, grid_depth = 100;
    Texture3D grid_texture(grid_width, grid_height, grid_depth, 10, Texture3D::debug_data(grid_width, grid_height, grid_depth));

    //
    // Render loop
    //
    while (!window.should_close())
    {
        if (ImGuiInstance::msaa) {
            msfb.bind();
        } else {
            fb.bind();
        }

        if (ImGuiInstance::wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        window.process_input();
        window.set_clear_color(ImGuiInstance::clear_r, ImGuiInstance::clear_g, ImGuiInstance::clear_b, 1.0f);
        window.clear();
    
        //spot_light.position = camera.position;
        //spot_light.direction = camera.front;

        //sphere.model = glm::translate(glm::mat4(1.0f), glm::vec3(glfwGetTime(), 0.0f, 0.0f));
        //model.model = glm::rotate(glm::mat4(1.0f), (float) glfwGetTime() * 0.02f, glm::vec3(0.0, 1.0, 0.0));

        scene.draw(&camera);

        if (ImGuiInstance::fsdebug) {
            fsdebug.draw(grid_texture, ImGuiInstance::fsdebug_scalar, {0.0, 0.0, 0.0}, {10.0, 10.0, 10.0});
        }

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
