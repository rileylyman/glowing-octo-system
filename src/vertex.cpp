#include <iostream>
#include <glad/glad.h>
#include "vertex.h"

void Vertex::setup_attrib_pointers() {
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));
    glEnableVertexAttribArray(1);
}

VertexBuffer::VertexBuffer() : filled(false) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
}

VertexBuffer::~VertexBuffer() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(ebos.size(), ebos.data());
}

uint32_t VertexBuffer::add_data(std::vector<Vertex> const& vertices, std::vector<uint32_t> & indices) {

    if (filled) {
        std::cout << "Cant add data to filled vertex buffer!" << std::endl;
        exit(EXIT_FAILURE);
    }
    glBindVertexArray(vao);

    for (int i = 0; i < indices.size(); i++) { indices[i] += unbuffered_data.size(); }

    uint32_t new_ebo;
    glGenBuffers(1, &new_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    ebos.push_back(new_ebo);
    unbuffered_data.insert(std::end(unbuffered_data), std::begin(vertices), std::end(vertices));

    return ebos.size() - 1;
}

void VertexBuffer::buffer_data() {
    if (filled) {
        std::cout << "Cant buffer already filled vertex buffer!" << std::endl;
        exit(EXIT_FAILURE);
    }

    filled = true;

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, unbuffered_data.size() * sizeof(Vertex), unbuffered_data.data(), GL_STATIC_DRAW);

    Vertex::setup_attrib_pointers();

}

void VertexBuffer::draw(uint32_t mesh_index, size_t count) const {

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[mesh_index]);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
}