#pragma once 

#include <glad/glad.h>
#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/kernel.h"
#include "engine/model.h"
#include <glm/glm.hpp>

struct FluidDebugRenderer {

private:
    Camera *camera;
    ShaderProgram draw_shader; 
    KernelProgram overlay_compute_shader; 
    uint32_t quad_vao, quad_vbo;

    float plane_width, plane_height, plane_z_offset;
    glm::vec3 grid_offset, grid_worldspace_whd;

public:
    FluidDebugRenderer(Camera *cam, float plane_width, float plane_height, float plane_z_offset, glm::vec3 grid_offset, glm::vec3 grid_worldspace_whd);


    void overlay_mask(Mask mask, Texture3D *grid);
    void draw(Texture3D grid, bool scalar);

    void plane_vectors(glm::vec3 *plane_x, glm::vec3 *plane_y); 

};

/*

Inputs:
    2 3D textures
        - Both RGBA32F
        - One is a mask 
        - Mask (0: solid, 1: vacuum, >=2: fluids)
        - Other is the velocity field (x,y,z)
    Offset for grid
    x,y,z scaling value
        
        -------------
        |   |   |   |   
        -------------
        |   |   |   |   
        -------------
        |   |   |   |   
        -------------
        |   |   |   |   
        -------------
        |   |   |   |   
        -------------
        |   |   |   |   
        -------------
*/

//void render_grid(Texture3D grid_mask, Texture3D q, bool scalar, glm::vec3 offset, glm::vec3 worldspace_whd);