#include "model.h"

Model::Model(
    VertexBuffer *vertex_buffer,
    std::vector<Vertex> vertices, 
    std::vector<uint32_t> indices, 
    ShaderProgram shader_prog,
    Camera *camera)
: vertex_buffer(vertex_buffer), shader_prog(shader_prog), indices_size(indices.size()), camera(camera)
{
    vertex_buffer_index = vertex_buffer->add_data(vertices, indices);
}

void Model::draw() {
    shader_prog.use();
    for (UniformName name : shader_prog.uniforms) {
        switch (name) {
            case VIEW:
                shader_prog.setMat4("view", camera->view());
            break;
            case MODEL_VIEW:
                shader_prog.setMat4("model_view", camera->view() * model);
            break;
            case NORMAL_MATRIX_VIEW:
                shader_prog.setMat3("normal_matrix_view", glm::mat3(camera->view()) * glm::transpose(glm::inverse(glm::mat3(model))));
            break;
            case TRANSFORM:
                shader_prog.setMat4("transform", camera->projection() * camera->view() * model);
            break;
            case MATERIAL:
                material.diffuse->use();
                material.specular->use();
                shader_prog.setInt("material.ambient", material.diffuse->unit);
                shader_prog.setInt("material.diffuse", material.diffuse->unit);
                shader_prog.setInt("material.specular", material.specular->unit);
                shader_prog.setFloat("material.shininess", material.shininess);
            break;
            case CONTAINER:
                //container->use();
                //shader_prog.setInt("container", container->unit);
            break;
            case SMILEY:
                //smiley->use();
                //shader_prog.setInt("smiley", smiley->unit);
            break;
        }
    }
    vertex_buffer->draw(vertex_buffer_index, indices_size);
}
