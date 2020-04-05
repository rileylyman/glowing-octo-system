#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include "shader.h"
#include "texture.h"
#include "vertex.h"
#include "camera.h"
#include <glm/glm.hpp>
#include <map>

struct Material {
    //Texture ambient, diffuse, specular, normal, height;
    //float shininess;
    Texture albedo, normal, metallic, roughness, ao;
};

struct Mesh {
    Material material;
    glm::mat4 model;

    Mesh(VertexBuffer *vertex_buffer,
        std::vector<Vertex> vertices, 
        std::vector<uint32_t> indices,
        glm::mat4 model,
        std::vector<Texture> ambient_textures,
        std::vector<Texture> diffuse_textures,
        std::vector<Texture> specular_textures,
        std::vector<Texture> normal_textures,
        std::vector<Texture> height_textures);
    void draw(ShaderProgram shader_prog, Camera *camera);

private:
    VertexBuffer *vertex_buffer;
    uint32_t vertex_buffer_index;
    size_t indices_size;
    std::vector<Texture> ambient_textures;
    std::vector<Texture> diffuse_textures;
    std::vector<Texture> specular_textures;
    std::vector<Texture> normal_textures;
    std::vector<Texture> height_textures;
};

struct Model {
    Model(VertexBuffer *vertex_buffer, std::string pathname, bool height_normals);
    void draw(ShaderProgram shader_prog, Camera *camera);

    glm::mat4 model = glm::mat4(1.0f);
    std::vector<Mesh> meshes;
private:
    bool height_normals = false;
    VertexBuffer *vertex_buffer;
    std::string directory;
    static std::map<std::string, Texture> loaded_textures;
    uint32_t unit = 0;

    void load_model(std::string pathname);
    Mesh process_mesh(aiMesh *ai_mesh, const aiScene *scene, glm::mat4 transformation); 
    void process_node(aiNode *node, const aiScene* scene, glm::mat4);
    std::vector<Texture> load_texture(aiMaterial *material, aiTextureType type, bool srgb);
};