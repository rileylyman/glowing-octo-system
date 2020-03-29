#include "model.h"

Model::Model(
    VertexBuffer *vertex_buffer,
    std::vector<Vertex> vertices, 
    std::vector<uint32_t> indices, 
    ShaderProgram shader_prog)
: vertex_buffer(vertex_buffer), shader_prog(shader_prog), indices_size(indices.size())
{
    vertex_buffer_index = vertex_buffer->add_data(vertices, indices);
}

void Model::draw() {
    shader_prog.use();
    for (UniformName name : shader_prog.uniforms) {
        switch (name) {
            case TRANSFORM:
                shader_prog.setMat4("transform", mvp);
            break;
            case OBJECT_COLOR:
                shader_prog.setVec3("objectColor", object_color);
            break;
            case LIGHT_COLOR:
                shader_prog.setVec3("lightColor", light_color);
            break;
            case CONTAINER:
                container->use();
                shader_prog.setInt("container", container->unit);
            break;
            case SMILEY:
                smiley->use();
                shader_prog.setInt("smiley", smiley->unit);
            break;
        }
    }
    vertex_buffer->draw(vertex_buffer_index, indices_size);
}
