#pragma once
#include <vector>
#include "shader.h"
#include "texture.h"
#include "vertex.h"
#include <glm/glm.hpp>
#include <map>

struct Model {
    Model(
        VertexBuffer *vertex_buffer,
        std::vector<Vertex> vertices, 
        std::vector<uint32_t> indices, 
        ShaderProgram shader_prog 
       );
    void draw();

    glm::mat4 mvp, model;
    glm::vec3 object_color, light_color;
    Texture *container, *smiley;

private:
    VertexBuffer *vertex_buffer;
    uint32_t vertex_buffer_index;
    size_t indices_size;

    ShaderProgram shader_prog;
};