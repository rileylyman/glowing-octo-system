#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 tex_coord;

    static void setup_attrib_pointers();
};

struct VertexBuffer {

    static void init_pbos(uint32_t w, uint32_t h, uint32_t d);

    bool filled;
    uint32_t vbo, vao;
    std::vector<uint32_t> ebos;

    std::vector<Vertex> unbuffered_data;

    VertexBuffer();
    ~VertexBuffer();

    uint32_t add_data(std::vector<Vertex> const& vertices, std::vector<uint32_t> & indices);
    void buffer_data();
    void draw(uint32_t mesh_index, size_t count) const; 
};
