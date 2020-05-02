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

public:
    FluidDebugRenderer(Camera *cam, float plane_width, float plane_height, float plane_z_offset)
    : shader(ShaderProgram("src/shaders/fsdebug.vert", "src/shaders/fsdebug.frag")), camera(cam) 
    {
        glm::mat4 camera_to_world = glm::inverse(cam->view());

        glm::vec4 camera_space_lower_corner = {-plane_width / 2.0, -plane_height / 2.0, plane_z_offset, 1.0f};
        glm::vec4 camera_space_upper_corner = { plane_width / 2.0,  plane_height / 2.0, plane_z_offset, 1.0f};

        glm::vec4 world_space_lower_corner = camera_to_world * camera_space_lower_corner; // plane lower left corner in world space
        glm::vec4 world_space_upper_corner = camera_to_world * camera_space_upper_corner; // plane upper right corner in world space


        float z = camera_space_lower_corner.z;
        float quad[] = {
            camera_space_lower_corner.x, camera_space_lower_corner.y, z,  
            camera_space_upper_corner.x, camera_space_upper_corner.y, z,  
            camera_space_lower_corner.x, camera_space_upper_corner.y, z,

            camera_space_lower_corner.x, camera_space_lower_corner.y, z,  
            camera_space_upper_corner.x, camera_space_lower_corner.y, z,  
            camera_space_upper_corner.x, camera_space_upper_corner.y, z,
        };

        glGenVertexArrays(1, &quad_vao);
        glBindVertexArray(quad_vao);

        glGenBuffers(1, &quad_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_VERTEX_ARRAY, 0);
    }

    void draw() {
        shader.use();
        shader.setMat4("u_Projection", camera->projection());
        glBindVertexArray(quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

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