#pragma once

#include <glm/glm.hpp>

#include <engine/kernel.h>

namespace Fluidsim {

class Engine {
    // DECLARE SHADERS
public:
    KernelProgram fs_advect_diffuse;
    KernelProgram fs_advect_diffuse_free;
    KernelProgram fs_advect_diffuse_mc;
    KernelProgram fs_apply_force;
    KernelProgram fs_div;
    KernelProgram fs_jacobi_iter;
    KernelProgram fs_pressure_proj;
    KernelProgram fs_write_to;

    // DECLARE GRID SIZE
    uint32_t grid_width, grid_height, grid_depth;
    float sclx, scly, sclz;

    // DECLARE MAXIMUM JACOBI ITERATIONS
    int max_iterations = 5;

    // DECLARE TEXTURES
    Texture3D u;
    Texture3D world_mask;
    Texture3D lin_buffer;
    Texture3D nearest_buffer;
    Texture3D zero;
    Texture3D q;
    Texture3D forces;
    Texture3D divq;
    Texture3D pres;
    Texture3D lin_buffer2;

    Engine(uint32_t w, uint32_t h, uint32_t d, float dx, float dy, float dz);

    void step(float dt);

    void fluidsim_testing123();

};

}