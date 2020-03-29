#include "model.h"

Model::Model(
    VertexBuffer *vertex_buffer,
    std::vector<Vertex> vertices, 
    std::vector<uint32_t> indices, 
    ShaderProgram shader_prog, 
    std::map<const char *, Texture> textures,
    glm::mat4 mvp) 
: vertex_buffer(vertex_buffer), shader_prog(shader_prog), mvp(mvp), textures(textures), indices_size(indices.size())
{
    vertex_buffer_index = vertex_buffer->add_data(vertices, indices);
}

void Model::draw() {
    shader_prog.use();
    for (auto it = textures.begin(); it != textures.end(); it++) {
        it->second.use();
        shader_prog.setInt(it->first, it->second.unit);
    }
    shader_prog.setMat4("transform", mvp);

    vertex_buffer->draw(vertex_buffer_index, indices_size);
}

void Model::set_mvp(glm::mat4 new_mvp) { mvp = new_mvp; }
