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

struct Mesh {
    BlinnPhongMaterial get_blinnphong_material();
    PBRMaterial get_pbr_material();
    glm::mat4 model;

    Mesh(VertexBuffer *vertex_buffer,
        std::vector<Vertex> vertices, 
        std::vector<uint32_t> indices,
        glm::mat4 model,
        std::map<TextureType, Texture> texmap);
    void draw();

private:
    bool blinnphong_cached = false;
    BlinnPhongMaterial bpmat;
    bool pbr_cached = false;
    PBRMaterial pbrmat;

    VertexBuffer *vertex_buffer;
    uint32_t vertex_buffer_index;
    size_t indices_size;
    std::map<TextureType, Texture> texmap;
};

struct Model {
    Model(VertexBuffer *vertex_buffer, std::string pathname, bool pbr, bool height_normals=false);
    void draw(ShaderProgram shader_prog, Camera *camera);

    glm::mat4 model = glm::mat4(1.0f);
    std::vector<Mesh> meshes;
private:
    bool pbr;
    bool height_normals = false;
    VertexBuffer *vertex_buffer;
    std::string directory;
    static std::map<std::string, Texture> loaded_textures;
    uint32_t unit = 0;

    glm::mat4 convert_matrix(const aiMatrix4x4 &aiMat); 
    void load_model(std::string pathname);
    Mesh process_mesh(aiMesh *ai_mesh, const aiScene *scene, glm::mat4 transformation); 
    void process_node(aiNode *node, const aiScene* scene, glm::mat4);
    Texture load_texture_from_name(std::string texname, bool srgb);
    std::vector<Texture> load_texture(aiMaterial *material, aiTextureType type, bool srgb);
};