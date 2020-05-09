#include <iostream>
#include "fluidsim/fluidsim.h"

#include <engine/texture.h>
#include <engine/framebuffer.h>
#include <engine/shader.h>
#include <glad/glad.h>

namespace Fluidsim {

void Engine::fluidsim_testing123() {

    Texture3D grid0(100, 100, 100, 0);
    Texture3D grid1(100, 100, 100, 1);

    Framebuffer framebuffer(100, 100);
    framebuffer.add_color_attachment();
    framebuffer.add_depth_stencil_attachment();
    framebuffer.bind();

    uint32_t cube_vao, cube_vbo;
    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);

    float data[] = {    
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    ShaderProgram shader("src/shaders/fluid.vert", "src/shaders/fluid.frag");
    shader.use();

    grid0.use();
    shader.setInt("u_Grid0", grid0.unit);
    
    grid1.use();
    shader.setInt("u_Grid1", grid1.unit);
    
    //shader.setFloat("u_Depth", 0.0f);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    framebuffer.unbind();

    uint32_t final_texture = framebuffer.get_color_texture();

    std::cout << "Hello from fluidsim!" << std::endl;
}

Engine::Engine(uint32_t w, uint32_t h, uint32_t d, float dx, float dy, float dz) {
    fs_apply_world_mask_overlay = KernelProgram("src/kernels/fs_apply_world_mask_overlay.comp");
    fs_advect_diffuse = KernelProgram("src/kernels/fs_advect_diffuse.comp");
    fs_advect_diffuse_free = KernelProgram("src/kernels/fs_advect_diffuse_free_surface.comp");
    fs_advect_mc = KernelProgram("src/kernels/fs_advect_maccormack.comp");
    fs_apply_force = KernelProgram("src/kernels/fs_apply_force.comp");
    fs_div = KernelProgram("src/kernels/fs_divergence.comp");
    fs_jacobi_iter = KernelProgram("src/kernels/fs_jacobi_iter_pressure_obstacle.comp");
    fs_pressure_proj = KernelProgram("src/kernels/fs_pressure_projection_obstacle.comp");
    fs_write_to = KernelProgram("src/kernels/fs_write_to.comp");

    grid_width = w;
    grid_height = h;
    grid_depth = d;

    sclx = dx;
    scly = dy;
    sclz = dz;

    u               = Texture3D(grid_width, grid_height, grid_depth, 1, Texture3D::zero(grid_width, grid_height, grid_depth));
    world_mask      = Texture3D(grid_width, grid_height, grid_depth, 2, Texture3D::world_mask(grid_width, grid_height, grid_depth), GL_NEAREST);
    lin_buffer      = Texture3D(grid_width, grid_height, grid_depth, 3, Texture3D::zero(grid_width, grid_height, grid_depth));
    nearest_buffer  = Texture3D(grid_width, grid_height, grid_depth, 4, Texture3D::zero(grid_width, grid_height, grid_depth), GL_NEAREST);
    zero            = Texture3D(grid_width, grid_height, grid_depth, 5, Texture3D::zero(grid_width, grid_height, grid_depth));
    q               = Texture3D(grid_width, grid_height, grid_depth, 6, Texture3D::q(grid_width, grid_height, grid_depth));
    forces          = Texture3D(grid_width, grid_height, grid_depth, 7, Texture3D::forces(grid_width, grid_height, grid_depth));
    temp            = Texture3D(grid_width, grid_height, grid_depth, 8, Texture3D::temperature(grid_width, grid_height, grid_depth));
    divq            = Texture3D(grid_width, grid_height, grid_depth, 9, Texture3D::zero(grid_width, grid_height, grid_depth));
    pres            = Texture3D(grid_width, grid_height, grid_depth, 10, Texture3D::zero(grid_width, grid_height, grid_depth));
    lin_buffer2     = Texture3D(grid_width, grid_height, grid_depth, 11, Texture3D::zero(grid_width, grid_height, grid_depth));
    temp_solid      = Texture3D(grid_width, grid_height, grid_depth, 12, Texture3D::temperatureSolid(grid_width, grid_height, grid_depth));
    prescpy[0]      = Texture3D(grid_width, grid_height, grid_depth, 13, Texture3D::zero(grid_width, grid_height, grid_depth));
    prescpy[1]      = Texture3D(grid_width, grid_height, grid_depth, 14, Texture3D::zero(grid_width, grid_height, grid_depth));
    prescpy[2]      = Texture3D(grid_width, grid_height, grid_depth, 15, Texture3D::zero(grid_width, grid_height, grid_depth));
}

void Engine::step(float dt, Texture3D *solid_mask, Texture3D *velocity_mask, Texture3D *temperature_mask, int max_steps) {

    const float timestep = 1.0 / 60.0;
    while (dt >= timestep && max_steps > 0) {
        step(timestep, solid_mask, velocity_mask, temperature_mask);
        dt -= timestep;
        max_steps--;
    }
}

void Engine::step(float dt, Texture3D *solid_mask, Texture3D *velocity_mask, Texture3D *temperature_mask) {

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
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // ADVANCE ITERATION
    iter += 1;
    if (iter % 3 == 0) iter = 0;

    // WRITE TO CURRENT SOLID MASK
    solid_mask->use(1, 1);
    world_mask.use(2, 2);
    nearest_buffer.use(3, 3);

    fs_apply_world_mask_overlay.use();
    fs_apply_world_mask_overlay.setInt("world_overlay", 1);
    fs_apply_world_mask_overlay.setInt("world_mask", 2);
    fs_apply_world_mask_overlay.setInt("world_mask_next", 3);

    glDispatchCompute((GLuint) grid_width, (GLuint) grid_depth, (GLuint) grid_height);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // WRITE WORLD_MASK_NEXT TO WORLD_MASK    
    fs_write_to.use();
    fs_write_to.setInt("q_in", 3);
    fs_write_to.setInt("q_out", 2);

    glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // ADVECTION
    u.use(1, 1);
    world_mask.use(2, 2);
    lin_buffer.use(3, 3);
    nearest_buffer.use(4, 4);
    velocity_mask->use(5, 5);
    q.use(6, 6);
    lin_buffer2.use(7, 7);

    // // TODO: PROPER FREE SURFACE ADVECTION
    // // ADVECT THE WORLD_MASK FIELD
    // fs_advect_diffuse_free.use();
    // fs_advect_diffuse_free.setInt("u", 1);
    // fs_advect_diffuse_free.setInt("world_mask", 2);
    // fs_advect_diffuse_free.setInt("world_mask_next", 4);
    // fs_advect_diffuse_free.setFloat("dt", dt);
    // fs_advect_diffuse_free.setVec3("scale", sclx, scly, sclz);

    // glDispatchCompute((GLuint) grid_width, (GLuint) grid_depth, (GLuint) grid_height);
    // glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // ADVECT SOME VELOCITY FIELD
    velocity_mask->use(5, 5);

    fs_advect_mc.use();
    fs_advect_mc.setInt("u", 1);
    fs_advect_mc.setInt("q_prev", 1);
    fs_advect_mc.setInt("q_solid", 5);
    fs_advect_mc.setInt("q_next", 3);
    fs_advect_mc.setInt("world_mask", 2);
    fs_advect_mc.setFloat("dt", dt);
    fs_advect_mc.setVec3("scale", sclx, scly, sclz);
    fs_advect_mc.setVec4("q_air", 0.0f, 0.0f, 0.0f, 0.0f);

    glDispatchCompute((GLuint) grid_width, (GLuint) grid_depth, (GLuint) grid_height);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    
    zero.use(5, 5);

    // ADVECT SOME QUANTITY Q
    fs_advect_mc.use();
    fs_advect_mc.setInt("u", 1);
    fs_advect_mc.setInt("q_prev", 6);
    fs_advect_mc.setInt("q_solid", 5);
    fs_advect_mc.setInt("q_next", 7);
    fs_advect_mc.setInt("world_mask", 2);
    fs_advect_mc.setFloat("dt", dt);
    fs_advect_mc.setVec3("scale", sclx, scly, sclz);
    fs_advect_mc.setVec4("q_air", 0.0f, 0.0f, 0.0f, 0.0f);

    glDispatchCompute((GLuint) grid_width, (GLuint) grid_depth, (GLuint) grid_height);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // WRITE Q_NEXT TO Q
    fs_write_to.use();
    fs_write_to.setInt("q_in", 7);
    fs_write_to.setInt("q_out", 6);

    glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // ADVECT TEMPERATURE FIELD
    temp.use(6, 6);
    temperature_mask->use(4, 4);

    fs_advect_diffuse.use();
    fs_advect_diffuse.setInt("u", 1);
    fs_advect_diffuse.setInt("q_prev", 6);
    fs_advect_diffuse.setInt("q_solid", 4);
    fs_advect_diffuse.setInt("q_next", 7);
    fs_advect_diffuse.setInt("world_mask", 2);
    fs_advect_diffuse.setFloat("dt", dt);
    fs_advect_diffuse.setVec3("scale", sclx, scly, sclz);
    fs_advect_diffuse.setVec4("q_air", 293.15f, 0.0f, 0.0f, 0.0f);

    glDispatchCompute((GLuint) grid_width, (GLuint) grid_depth, (GLuint) grid_height);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // WRITE NEXT_TEMP TO TEMP
    fs_write_to.use();
    fs_write_to.setInt("q_in", 7);
    fs_write_to.setInt("q_out", 6);

    glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    nearest_buffer.use(4, 4);

    // // WRITE WORLD_MASK_NEXT TO WORLDMASK
    // fs_write_to.use();
    // fs_write_to.setInt("q_in", 4);
    // fs_write_to.setInt("q_out", 2);

    // glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
    // glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // WRITE W_NEXT TO U    
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
    temp.use(6, 6);
    pres.use(7, 7);

    fs_apply_force.use();
    fs_apply_force.setInt("w", 1);
    fs_apply_force.setInt("f", 5);

    fs_apply_force.setInt("temp", 6);
    fs_apply_force.setInt("pressure", 7);

    fs_apply_force.setFloat("rho", 1.0f);
    fs_apply_force.setFloat("g", -9.8f);

    fs_apply_force.setFloat("temp_air", 293.15f + 100.0f);

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
    velocity_mask->use(4, 4);
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

    for(int iter = 0; iter < max_iterations; iter++) {
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
    fs_pressure_proj.setInt("u_solid", 4);
    fs_pressure_proj.setInt("world_mask", 2);

    glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // WRITE TO NEW VALUES
    fs_write_to.use();
    fs_write_to.setInt("q_in", 3);
    fs_write_to.setInt("q_out", 1);

    glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    
    // WRITE TO PRESSURE BUFFER
    if (iter % 3 == 0) {
        prescpy[0].use(7, 7);
    } else if (iter % 3 == 1) {
        prescpy[1].use(7, 7);
    } else {
        prescpy[2].use(7, 7);
    }

    fs_write_to.use();
    fs_write_to.setInt("q_in", 6);
    fs_write_to.setInt("q_out", 7);

    glDispatchCompute((GLuint) grid_width, (GLuint) grid_height, (GLuint) grid_depth);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

}
