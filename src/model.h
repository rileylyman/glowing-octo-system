#pragma once
#include <vector>
#include "shader.h"
#include "texture.h"
#include "vertex.h"
#include <map>

struct Model {
    Model(
        VertexBuffer *vertex_buffer,
        std::vector<Vertex> vertices, 
        std::vector<uint32_t> indices, 
        ShaderProgram shader_prog, 
        std::map<const char *, Texture> textures,
        glm::mat4 mvp
        );
    void draw();
    void set_mvp(glm::mat4 new_mvp);
    glm::mat4 mvp;

private:
    VertexBuffer *vertex_buffer;
    uint32_t vertex_buffer_index;
    size_t indices_size;

    ShaderProgram shader_prog;
    std::map<const char*, Texture> textures;

};