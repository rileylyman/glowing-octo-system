#pragma once
#include <vector>
#include "shader.h"
#include "texture.h"
#include "vertex.h"
#include "camera.h"
#include <glm/glm.hpp>
#include <map>

struct Material {
    Texture *diffuse, *specular;
    float shininess;
};

struct Model {
    Model(
        VertexBuffer *vertex_buffer,
        std::vector<Vertex> vertices, 
        std::vector<uint32_t> indices, 
        ShaderProgram shader_prog,
        Camera *camera
       );
    void draw();

    glm::mat4 mvp, model;
    Material material;

    ShaderProgram shader_prog;
private:
    VertexBuffer *vertex_buffer;
    uint32_t vertex_buffer_index;
    size_t indices_size;
    Camera *camera;

};