#pragma once

#include <glm/glm.hpp>

#include <engine/kernel.h>

namespace Fluidsim {

class Engine {
    // DECLARE SHADERS
public:
    KernelProgram fs_apply_overlay;
    KernelProgram fs_advect_diffuse;
    KernelProgram fs_advect_diffuse_free;
    KernelProgram fs_advect_mc;
    KernelProgram fs_apply_force;
    KernelProgram fs_div;
    KernelProgram fs_jacobi_iter;
    KernelProgram fs_pressure_proj;
    KernelProgram fs_write_to;

    // DECLARE GRID SIZE
    uint32_t grid_width, grid_height, grid_depth;
    float sclx, scly, sclz;

    // DECLARE MAXIMUM JACOBI ITERATIONS * 2
    int max_iterations = 10;
    
    // DECLARE COUNTER FOR WRITING PIXEL BUFFERS
    int iter = 0;

    // DECLARE TEXTURES
    Texture3D u;
    Texture3D world_mask;
    Texture3D lin_buffer;
    Texture3D nearest_buffer;
    Texture3D zero;
    Texture3D q;
    Texture3D forces;
    Texture3D temp;
    Texture3D temp_solid;
    Texture3D divq;
    Texture3D pres;
    Texture3D lin_buffer2;
    Texture3D prescpy[3];

    Engine(uint32_t w, uint32_t h, uint32_t d, float dx, float dy, float dz);

    void step(float dt, Texture3D *solidmask);
    void step(float dt, Texture3D *solidmask, int max_steps); 

    void fluidsim_testing123();

};

}