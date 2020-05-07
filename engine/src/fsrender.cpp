#include <iostream>
#include "engine/fsrender.h"

FluidDebugRenderer::FluidDebugRenderer(Camera *cam, float plane_width, float plane_height, float plane_z_offset, glm::vec3 grid_offset, glm::vec3 grid_worldspace_whd)
: draw_shader(ShaderProgram("src/shaders/fsdebug.vert", "src/shaders/fsdebug.frag")), 
overlay_compute_shader(KernelProgram("src/kernels/fs_overlay.comp")),
camera(cam), 
plane_width(plane_width), 
plane_height(plane_height), 
plane_z_offset(plane_z_offset),
grid_offset(grid_offset),
grid_worldspace_whd(grid_worldspace_whd)
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

void FluidDebugRenderer::plane_vectors(glm::vec3 *plane_x, glm::vec3 *plane_y) {
    glm::mat4 camera_to_world = glm::inverse(camera->view());

    glm::vec4 camera_space_bottom_left_corner = {-plane_width / 2.0, -plane_height / 2.0, plane_z_offset, 1.0f};
    glm::vec4 camera_space_top_left_corner = {-plane_width / 2.0, plane_height / 2.0, plane_z_offset, 1.0f};
    glm::vec4 camera_space_bottom_right_corner = {plane_width / 2.0, -plane_height / 2.0, plane_z_offset, 1.0f};

    glm::vec3 bl = camera_to_world * camera_space_bottom_left_corner;
    glm::vec3 ul = camera_to_world * camera_space_top_left_corner;
    glm::vec3 br = camera_to_world * camera_space_bottom_right_corner;

    *plane_x = br - bl;
    *plane_y = ul - bl;

}

void FluidDebugRenderer::overlay_mask(Mask mask, Texture3D *grid, glm::vec3 value_to_write) {

    grid->use(1, 1);
    mask.tex.use(2, 2);

    overlay_compute_shader.use();
    overlay_compute_shader.setVec3("u_ValueToWrite", value_to_write);
    overlay_compute_shader.setVec3("u_Least", glm::vec4(mask.bbox_least, 1.0));
    overlay_compute_shader.setVec3("u_Most",  glm::vec4(mask.bbox_most, 1.0));
    overlay_compute_shader.setMat4("u_InverseWorld", glm::inverse(mask.parent->model() * mask.bind_matrix));
    overlay_compute_shader.setInt("u_Grid", 1);
    overlay_compute_shader.setInt("u_Mask", 2);
    overlay_compute_shader.setVec3("u_GridOffset", grid_offset);
    overlay_compute_shader.setVec3("u_GridDimensions", grid_worldspace_whd);
    overlay_compute_shader.setVec3("u_GridNumCells", (float)grid->width, (float)grid->height, (float)grid->depth);
    overlay_compute_shader.setVec3("u_MaskNumCells", (float)mask.tex.width, (float)mask.tex.height, (float)mask.tex.depth);

    glDispatchCompute((GLuint) grid->width, (GLuint) grid->depth, (GLuint) grid->height);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void FluidDebugRenderer::draw(Texture3D grid, bool scalar) {

    glm::vec3 plane_x, plane_y;
    plane_vectors(&plane_x, &plane_y);

    draw_shader.use();
    grid.use();
    draw_shader.setInt("u_Grid", grid.unit);
    draw_shader.setBool("u_Scalar", scalar);
    draw_shader.setVec3("u_Offset", grid_offset);
    draw_shader.setVec3("u_PlaneX", plane_x);
    draw_shader.setVec3("u_PlaneY", plane_y);
    draw_shader.setVec3("u_Dimensions", grid_worldspace_whd);
    draw_shader.setMat4("u_Projection", camera->projection());
    draw_shader.setMat4("u_InverseView", glm::inverse(camera->view()));
    glBindVertexArray(quad_vao);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}