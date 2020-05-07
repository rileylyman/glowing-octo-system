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
#include "engine/debug.h"

#include <fluidsim/fluidsim.h>
#include "engine/kernel.h"

#include <iostream>

Model construct_sphere(VertexBuffer *, float, int, int, MeshShaderType);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
    uint32_t grid_width = 32, grid_height = 32, grid_depth = 32;

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

    Physics *physics = new Physics();
    VertexBuffer::init_pbos(grid_width, grid_height, grid_depth);

    //
    // Set up imgui instance
    //
    ImGuiInstance imgui_instance(window.window, &camera.position);
    
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
    Scene scene("src/scenes/test.json", &vertex_buffer);
    vertex_buffer.buffer_data();

    //
    // Init Fluidsim
    //
    Fluidsim::Engine fs(grid_width, grid_height, grid_depth, 0.25f, 0.25f, 0.25f);
    Physics::instance->fs = &fs;

    glm::vec3 grid_offset = glm::vec3(0.0);
    FluidDebugRenderer fsdebug(&camera, 10.0f, 5.0f, -10.0f, grid_offset, {20.0, 20.0, 20.0});    

    std::vector<Mask> mesh_masks = scene.get_mesh_masks();
    
    Texture3D output_solid_mask(grid_width, grid_height, grid_depth, 0, Texture3D::zero(grid_width, grid_height, grid_depth), GL_NEAREST);
    Texture3D output_velocity_mask(grid_width, grid_height, grid_depth, 0, Texture3D::zero(grid_width, grid_height, grid_depth), GL_NEAREST);
    Texture3D output_temperature_mask(grid_width, grid_height, grid_depth, 0, Texture3D::zero(grid_width, grid_height, grid_depth), GL_NEAREST);

    Texture3D zero = Texture3D(grid_width, grid_height, grid_depth, 5, Texture3D::zero(grid_width, grid_height, grid_depth));


    //
    // Render loop
    //

    int work_grp_cnt[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

    printf("max global (total) work group counts x:%i y:%i z:%i\n",
    work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

    GLint work_grp_inv;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
    printf("max local work group invocations %i\n", work_grp_inv);

    
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

        glCheckError();
        for (Model &model : scene.get_models()) {
            //model.physics_obj->apply_force_to_center({0.0, 0.0, -1.0});
            model.pressure_force(fs, 4, 2, grid_offset, model.model());
        }
        scene.draw(&camera);

        //
        // Fluid Simulation (TODO: move to scene.draw)
        //

        // Zero the World Mask
        zero.use(1, 1);
        output_solid_mask.use(2, 2);
        fs.fs_write_to.use();
        fs.fs_write_to.setInt("q_in", 1);
        fs.fs_write_to.setInt("q_out", 2);

        glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        // Get Objects in the Worldview and Stick into World Mask
        for (Mask &mask : mesh_masks) {
            glm::vec3 velocity = mask.parent->physics_obj->get_velocity();
            fsdebug.overlay_mask(mask, &output_solid_mask, glm::vec3(1.0, 1.0, 1.0));
            fsdebug.overlay_mask(mask, &output_velocity_mask, velocity);
            fsdebug.overlay_mask(mask, &output_temperature_mask, glm::vec3(300., 0.0, 0.0));
        }

        // Fluid Physics
        if (ImGuiInstance::physics_enabled) {
            double current_time = glfwGetTime();
            double frame_time = current_time - Physics::instance->previous_time;
            Physics::instance->previous_time = current_time;
            fs.step(frame_time, &output_solid_mask, 10);
            Physics::instance->tick(frame_time);
        }

        // Fluid Debugger
        if (ImGuiInstance::mask_overlay) {
            fsdebug.draw(output_solid_mask, ImGuiInstance::fsdebug_scalar);
        } else if (ImGuiInstance::fluid_overlay) {
            fsdebug.draw(fs.q, ImGuiInstance::fsdebug_scalar);
        }
        
        //
        // End Fluid Simulation
        //

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
        return Model(vertex_buffer, vertices, indices, BlinnPhongSolidMaterial{ glm::vec3(0.3f), glm::vec3(0.6f), glm::vec3(1.0f), 64.0f}, RigidBodyType::DYNAMIC, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});
    } else if (shader_type == PBR_SOLID) {
        return Model(vertex_buffer, vertices, indices, PBRSolidMaterial{ glm::vec3(0.7f, 0.1f, 0.1f), 1.0f, 0.3f }, RigidBodyType::DYNAMIC, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});
    } else {
        throw "Sphere with this shader type is not yet supported";
    }
}
