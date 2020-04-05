#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "model.h"
#include "imgui-instance.h"

std::map<std::string, Texture> Model::loaded_textures = {};

Mesh::Mesh(
    VertexBuffer *vertex_buffer,
    std::vector<Vertex> vertices, 
    std::vector<uint32_t> indices,
    glm::mat4 model,
    std::vector<Texture> ambient_textures,
    std::vector<Texture> diffuse_textures,
    std::vector<Texture> specular_textures,
    std::vector<Texture> normal_textures,
    std::vector<Texture> height_textures)
: vertex_buffer(vertex_buffer), indices_size(indices.size()), diffuse_textures(diffuse_textures), specular_textures(specular_textures),
normal_textures(normal_textures), height_textures(height_textures), ambient_textures(ambient_textures), model(model)
{
    vertex_buffer_index = vertex_buffer->add_data(vertices, indices);
    if (diffuse_textures.empty()) {
        if (ambient_textures.empty()) {
            std::cout << "Must at least have a diffuse or ambient texture!\n";
            exit(EXIT_FAILURE);
        }
        material.ambient = ambient_textures[0]; 
        material.diffuse = ambient_textures[0];
        material.height = height_textures.size() > 0 ? height_textures[0] : ambient_textures[0];
        material.normal = normal_textures.size() > 0 ? normal_textures[0] : ambient_textures[0];
        material.specular = specular_textures.size() > 0 ? specular_textures[0] : ambient_textures[0];
    } else {
        material.ambient = ambient_textures.size() > 0 ? ambient_textures[0] : diffuse_textures[0];
        material.diffuse = diffuse_textures[0];
        material.height = height_textures.size() > 0 ? height_textures[0] : diffuse_textures[0];
        material.normal = normal_textures.size() > 0 ? normal_textures[0] : diffuse_textures[0];
        material.specular = specular_textures.size() > 0 ? specular_textures[0] : diffuse_textures[0];
    }
    material.shininess = 64.0f;
}

void Mesh::draw(ShaderProgram shader_prog, Camera *camera) {
    shader_prog.use();
    for (UniformName name : shader_prog.uniforms) {
        switch (name) {
            case CAMERA_POS:
                shader_prog.setVec3("camera_pos", camera->position);
            break;
            case MODEL:
                shader_prog.setMat4("model", model);
            break;
            case NORMAL_MATRIX:
                shader_prog.setMat3("normal_matrix", glm::transpose(glm::inverse(glm::mat3(model))));
            break;
            case TRANSFORM:
                shader_prog.setMat4("transform", camera->projection() * camera->view() * model);
            break;
            case MATERIAL:
                material.ambient.use();
                material.diffuse.use();
                material.specular.use();
                material.normal.use();
                material.height.use();
                shader_prog.setInt("material.ambient", material.ambient.unit);
                shader_prog.setInt("material.diffuse", material.diffuse.unit);
                shader_prog.setInt("material.specular", material.specular.unit);
                shader_prog.setInt("material.normal", material.normal.unit);
                shader_prog.setInt("material.height", material.height.unit);
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

Model::Model(VertexBuffer *vertex_buffer, std::string pathname, bool height_normals) 
: vertex_buffer(vertex_buffer), height_normals(height_normals)
{
    load_model(pathname);
}

void Model::draw(ShaderProgram shader_prog, Camera *camera) {
    shader_prog.setBool("u_RenderNormals", ImGuiInstance::render_normals);
    for (Mesh mesh : meshes) {
        mesh.draw(shader_prog, camera);
    }
}

glm::mat4 convert_matrix(const aiMatrix4x4 &aiMat) {
    return {
        aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
        aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
        aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
        aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
    };
}

void Model::load_model(std::string pathname) {
    
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(pathname, aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_GenNormals);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "Assimp importer error: " << importer.GetErrorString() << std::endl;
        exit(EXIT_FAILURE);
    }
    directory = pathname.substr(0, pathname.find_last_of('/'));

    process_node(scene->mRootNode, scene, glm::mat4(1.0f));
}

void Model::process_node(aiNode *node, const aiScene *scene, glm::mat4 transformation) {
    glm::mat4 current_tr = convert_matrix(node->mTransformation);
    transformation = transformation * current_tr  ;

    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh *ai_mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh mesh = process_mesh(ai_mesh, scene, transformation);
        meshes.push_back(mesh);
    }
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene, transformation);
    }
}

Mesh Model::process_mesh(aiMesh *ai_mesh, const aiScene *scene, glm::mat4 transformation) {

    unit = 0;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<Texture> ambient_textures;
    std::vector<Texture> diffuse_textures;
    std::vector<Texture> specular_textures;
    std::vector<Texture> normal_textures;
    std::vector<Texture> height_textures;

    for (uint32_t i = 0; i < ai_mesh->mNumVertices; i++) {
        Vertex vertex;

        aiVector3D ai_position = ai_mesh->mVertices[i];
        aiVector3D ai_normal = ai_mesh->mNormals[i];
        aiVector3D ai_tangent = ai_mesh->mTangents[i];
        aiVector3D ai_bitangent = ai_mesh->mBitangents[i];
        
        vertex.position = glm::vec3(ai_position.x, ai_position.y, ai_position.z);
        vertex.normal = glm::vec3(ai_normal.x, ai_normal.y, ai_normal.z);
        vertex.tangent = glm::vec3(ai_tangent.x, ai_tangent.y, ai_tangent.z);
        vertex.bitangent = glm::vec3(ai_bitangent.x, ai_bitangent.y, ai_bitangent.z);
        vertex.tex_coord = glm::vec2(0.0f, 0.0f);
        if (ai_mesh->mTextureCoords[0]) {
            aiVector3D ai_tex_coord = ai_mesh->mTextureCoords[0][i];
            vertex.tex_coord.x = ai_tex_coord.x;
            vertex.tex_coord.y = ai_tex_coord.y;
        } else {
            exit(EXIT_FAILURE);
        }

        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < ai_mesh->mNumFaces; i++) {
        aiFace face = ai_mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    aiMaterial *material = scene->mMaterials[ai_mesh->mMaterialIndex];
    ambient_textures = load_texture(material, aiTextureType_AMBIENT, true);
    diffuse_textures = load_texture(material, aiTextureType_DIFFUSE, true);
    specular_textures = load_texture(material, aiTextureType_SPECULAR, true);
    normal_textures = load_texture(material, height_normals ? aiTextureType_HEIGHT : aiTextureType_NORMALS, true);
    height_textures = load_texture(material, aiTextureType_HEIGHT, false);

    return Mesh(
        vertex_buffer,
        vertices,
        indices,
        model * transformation,
        ambient_textures,
        diffuse_textures,
        specular_textures,
        normal_textures,
        height_textures
    );
}

std::vector<Texture> Model::load_texture(aiMaterial *material, aiTextureType type, bool srgb) {
    std::vector<Texture> textures;
    for(uint32_t i = 0; i < material->GetTextureCount(type); i++) {

        aiString str;
        material->GetTexture(type, i, &str);

        std::string texname = std::string(str.C_Str());
        if (loaded_textures.count(texname) > 0) {
            textures.push_back(loaded_textures.at(texname));
        } else {
            if (unit >= 16) {
                std::cout << "Can have at most 16 textures for same model!\n";
                exit(EXIT_FAILURE);
            }
            Texture texture(directory + "/" + texname, unit++, srgb);
            textures.push_back(texture);
            loaded_textures[texname] = texture;
            std::cout << "Got " << texname << std::endl;
        }
    }
    return textures;
}
