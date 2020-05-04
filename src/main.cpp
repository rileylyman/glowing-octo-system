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
    Physics::init();
    // fluidsim_testing123();

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

    // DECLARE SHADERS
    KernelProgram fs_advect_diffuse("src/kernels/fs_advect_diffuse.comp");
    KernelProgram fs_advect_diffuse_free("src/kernels/fs_advect_diffuse_free_surface.comp");
    KernelProgram fs_apply_force("src/kernels/fs_apply_force.comp");
    KernelProgram fs_div("src/kernels/fs_divergence.comp");
    KernelProgram fs_jacobi_iter("src/kernels/fs_jacobi_iter_pressure_obstacle.comp");
    KernelProgram fs_pressure_proj("src/kernels/fs_pressure_projection_obstacle.comp");
    KernelProgram fs_write_to("src/kernels/fs_write_to.comp");
    
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

    FluidDebugRenderer fsdebug(&camera, 10.0f, 5.0f, -10.0f);    
    uint32_t grid_width = 75, grid_height = 75, grid_depth = 75;

    Texture3D u(grid_width, grid_height, grid_depth, 1, Texture3D::u(grid_width, grid_height, grid_depth));
    Texture3D world_mask(grid_width, grid_height, grid_depth, 2, Texture3D::world_mask(grid_width, grid_height, grid_depth), GL_NEAREST);
    Texture3D lin_buffer(grid_width, grid_height, grid_depth, 3, Texture3D::zero(grid_width, grid_height, grid_depth));
    Texture3D nearest_buffer(grid_width, grid_height, grid_depth, 4, Texture3D::zero(grid_width, grid_height, grid_depth), GL_NEAREST);
    Texture3D zero(grid_width, grid_height, grid_depth, 5, Texture3D::zero(grid_width, grid_height, grid_depth));
    Texture3D q(grid_width, grid_height, grid_depth, 6, Texture3D::q(grid_width, grid_height, grid_depth));
    Texture3D forces(grid_width, grid_height, grid_depth, 7, Texture3D::zero(grid_width, grid_height, grid_depth));
    Texture3D divq(grid_width, grid_height, grid_depth, 8, Texture3D::zero(grid_width, grid_height, grid_depth));
    Texture3D pres(grid_width, grid_height, grid_depth, 9, Texture3D::zero(grid_width, grid_height, grid_depth));


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
        scene.draw(&camera);

        if (ImGuiInstance::fsdebug) {
            /**
             * Physics Steps:
             * 
             * (1) ADVECTION
             *      (a) VELOCITY
             *      (b) WORLD MASK
             *      
             * (2) DIFFUSION (NOT IMPLEMENTED)
             * 
             * (3) FORCE APPLICATION
             *      (a) VELOCITY
             * 
             * (4) PRESSURE PROJECTION
             *      (a) DIVERGENCE OF  UNPROJECTED VELOCITY
             *      (b) JACOBI ITERATIONS FOR PRESSURE
             *      (c) PROJECTION STEP FOR PRESSURE
            */

            // PARAMS
            float dt = 0.01f;
            float sclx = 0.2f;
            float scly = 0.2f;
            float sclz = 0.2f;

            // ADVECTION
            u.use(1, 1);
            world_mask.use(2, 2);
            lin_buffer.use(3, 3);
            nearest_buffer.use(4, 4);
            zero.use(5, 5);
            q.use(6, 6);

            // ADVECT THE WORLD_MASK FIELD
            fs_advect_diffuse_free.use();
            fs_advect_diffuse_free.setInt("u", 1);
            fs_advect_diffuse_free.setInt("world_mask", 2);
            fs_advect_diffuse_free.setInt("world_mask_next", 4);
            fs_advect_diffuse_free.setFloat("dt", dt);
            fs_advect_diffuse_free.setVec3("scale", sclx, scly, sclz);

            glDispatchCompute((GLuint) grid_width, (GLuint) grid_depth, (GLuint) grid_height);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // ADVECT SOME QUANTITY FIELD
            fs_advect_diffuse.use();
            fs_advect_diffuse.setInt("u", 1);
            fs_advect_diffuse.setInt("q_prev", 1);
            fs_advect_diffuse.setInt("q_solid", 5);
            fs_advect_diffuse.setInt("q_next", 3);
            fs_advect_diffuse.setInt("world_mask", 2);
            fs_advect_diffuse.setFloat("dt", dt);
            fs_advect_diffuse.setVec3("scale", sclx, scly, sclz);
            fs_advect_diffuse.setVec4("q_air", 0.0f, 0.0f, 0.0f, 0.0f);

            glDispatchCompute((GLuint) grid_width, (GLuint) grid_depth, (GLuint) grid_height);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // WRITE TO NEW VALUES
            fs_write_to.use();
            fs_write_to.setInt("q_in", 4);
            fs_write_to.setInt("q_out", 2);

            glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            fs_write_to.use();
            fs_write_to.setInt("q_in", 3);
            fs_write_to.setInt("q_out", 1);

            glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            
            // EXTERNAL FORCES
            u.use(1, 1);
            world_mask.use(2, 2);
            lin_buffer.use(3, 3);
            zero.use(4, 4);
            forces.use(5, 5);

            fs_apply_force.use();
            fs_apply_force.setInt("w", 1);
            fs_apply_force.setInt("f", 5);
            fs_apply_force.setFloat("rho", 1.0f);
            fs_apply_force.setFloat("g", -9.8f);
            fs_apply_force.setVec3("scale", sclx, scly, sclz);
            fs_apply_force.setFloat("dt", dt);
            fs_apply_force.setInt("w_next", 3);
            fs_apply_force.setInt("world_mask", 2);

            glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // WRITE TO NEW VALUES
            fs_write_to.use();
            fs_write_to.setInt("q_in", 3);
            fs_write_to.setInt("q_out", 1);

            glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            
            // APPLY DIVERGENCE TO W the JACOBBIIIBIBIBIBIBBIBBIBIBIBIBI
            u.use(1, 1);
            world_mask.use(2, 2);
            lin_buffer.use(3, 3);
            zero.use(4, 4);
            divq.use(5, 5);
            pres.use(6, 6);

            fs_div.use();
            fs_div.setInt("q", 1);
            fs_div.setInt("q_solid", 4);
            fs_div.setInt("div_q", 5);
            fs_div.setInt("world_mask", 2);

            glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // JACOBBBBBBIIIIIIIIIIII

            for(int iter = 0; iter < 1; iter++) {
                // JACOBOBBOBOIBSOFIBODFIBODFIBODBIBOIIIII
                fs_jacobi_iter.use();
                fs_jacobi_iter.setInt("pressure", 6);
                fs_jacobi_iter.setInt("div_w", 5);
                fs_jacobi_iter.setInt("pressure_next", 3);
                fs_jacobi_iter.setInt("world_mask", 2);
                fs_jacobi_iter.setFloat("pressure_air", 0.0f);

                glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
                glMemoryBarrier(GL_ALL_BARRIER_BITS);

                fs_jacobi_iter.use();
                fs_jacobi_iter.setInt("pressure", 3);
                fs_jacobi_iter.setInt("div_w", 5);
                fs_jacobi_iter.setInt("pressure_next", 6);
                fs_jacobi_iter.setInt("world_mask", 2);
                fs_jacobi_iter.setFloat("pressure_air", 0.0f);

                glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
                glMemoryBarrier(GL_ALL_BARRIER_BITS);
            }

            // Presure Projection Time!!!!
            fs_pressure_proj.use();
            fs_pressure_proj.setInt("w", 1);
            fs_pressure_proj.setInt("pressure", 6);
            fs_pressure_proj.setInt("u_next", 3);
            fs_pressure_proj.setInt("world_mask", 2);

            glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // WRITE TO NEW VALUES
            fs_write_to.use();
            fs_write_to.setInt("q_in", 3);
            fs_write_to.setInt("q_out", 1);

            glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);


            fsdebug.draw(u, ImGuiInstance::fsdebug_scalar, {0.0, 0.0, 0.0}, {16.0, 16.0, 16.0});
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
        return Model(vertex_buffer, vertices, indices, BlinnPhongSolidMaterial{ glm::vec3(0.3f), glm::vec3(0.6f), glm::vec3(1.0f), 64.0f}, RigidBodyType::DYNAMIC, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});
    } else if (shader_type == PBR_SOLID) {
        return Model(vertex_buffer, vertices, indices, PBRSolidMaterial{ glm::vec3(0.7f, 0.1f, 0.1f), 1.0f, 0.3f }, RigidBodyType::DYNAMIC, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});
    } else {
        throw "Sphere with this shader type is not yet supported";
    }
}
