#pragma once 

#include <glad/glad.h>
#include "camera.h"
#include "shader.h"
#include "texture.h"
#include <glm/glm.hpp>

struct FluidDebugRenderer {

private:
    Camera *camera;
    ShaderProgram shader; 
    uint32_t quad_vao, quad_vbo;

    float plane_width, plane_height, plane_z_offset;

public:
    FluidDebugRenderer(Camera *cam, float plane_width, float plane_height, float plane_z_offset);

    void draw(Texture3D grid, bool scalar, glm::vec3 offset, glm::vec3 worldspace_whd);

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