#include <glm/gtc/matrix_transform.hpp>
#include <assimp/pbrmaterial.h>
#include <iostream>
#include "model.h"
#include "imgui-instance.h"

std::map<std::string, Texture> Model::loaded_textures = {};

Mesh::Mesh(
    VertexBuffer *vertex_buffer,
    std::vector<Vertex> vertices, 
    std::vector<uint32_t> indices,
    glm::mat4 model,
    std::map<TextureType, Texture> texmap)
: vertex_buffer(vertex_buffer), indices_size(indices.size()), model(model), texmap(texmap)
{
    vertex_buffer_index = vertex_buffer->add_data(vertices, indices);
}

PBRMaterial Mesh::get_pbr_material() {
    if (pbr_cached) {
        return pbrmat;
    }
    PBRMaterial material;
    material.albedo = texmap[TEXTURE_TYPE_ALBEDO_MAP];
    material.metallic = texmap[TEXTURE_TYPE_METALLIC_MAP];
    material.ao = texmap[TEXTURE_TYPE_AO_MAP];
    material.roughness = texmap[TEXTURE_TYPE_ROUGHNESS_MAP];
    material.normal = texmap[TEXTURE_TYPE_NORMAL_MAP];

    return material;
}

BlinnPhongMaterial Mesh::get_blinnphong_material() {
    if (blinnphong_cached) {
        return bpmat;
    }

    BlinnPhongMaterial material;
    if (texmap.count(TEXTURE_TYPE_DIFFUSE_MAP) == 0) {
        if (texmap.count(TEXTURE_TYPE_AMBIENT_MAP) == 0) {
            std::cout << "Must at least have a diffuse or ambient texture!\n";
            exit(EXIT_FAILURE);
        }
        material.ambient = texmap[TEXTURE_TYPE_AMBIENT_MAP];
        material.diffuse = texmap[TEXTURE_TYPE_AMBIENT_MAP];
        material.height = texmap.count(TEXTURE_TYPE_HEIGHT_MAP) > 0 ? texmap[TEXTURE_TYPE_HEIGHT_MAP] : texmap[TEXTURE_TYPE_AMBIENT_MAP];
        material.normal = texmap.count(TEXTURE_TYPE_NORMAL_MAP) > 0 ? texmap[TEXTURE_TYPE_NORMAL_MAP] : texmap[TEXTURE_TYPE_AMBIENT_MAP];
        material.specular = texmap.count(TEXTURE_TYPE_SPECULAR_MAP) > 0 ? texmap[TEXTURE_TYPE_SPECULAR_MAP] : texmap[TEXTURE_TYPE_AMBIENT_MAP];
    } else {
        material.ambient = texmap.count(TEXTURE_TYPE_AMBIENT_MAP) > 0 ? texmap[TEXTURE_TYPE_AMBIENT_MAP] : texmap[TEXTURE_TYPE_DIFFUSE_MAP];
        material.diffuse = texmap[TEXTURE_TYPE_DIFFUSE_MAP];
        material.height = texmap.count(TEXTURE_TYPE_HEIGHT_MAP) > 0 ? texmap[TEXTURE_TYPE_HEIGHT_MAP] : texmap[TEXTURE_TYPE_DIFFUSE_MAP];
        material.normal = texmap.count(TEXTURE_TYPE_NORMAL_MAP) > 0 ? texmap[TEXTURE_TYPE_NORMAL_MAP] : texmap[TEXTURE_TYPE_DIFFUSE_MAP];
        material.specular = texmap.count(TEXTURE_TYPE_SPECULAR_MAP) > 0 ? texmap[TEXTURE_TYPE_SPECULAR_MAP] : texmap[TEXTURE_TYPE_DIFFUSE_MAP];
    }
    material.shininess = 64.0f;

    return material;
}

void Mesh::draw() {
    vertex_buffer->draw(vertex_buffer_index, indices_size);
}

Model::Model(VertexBuffer *vertex_buffer, std::string pathname, bool pbr, bool height_normals) 

: vertex_buffer(vertex_buffer), height_normals(height_normals), pbr(pbr)
{
    load_model(pathname);
}

void Model::draw(ShaderProgram shader_prog, Camera *camera) {
    throw "This will produce an incorrect result... for now";
    for (Mesh mesh : meshes) {
        mesh.draw();
    }
}

glm::mat4 Model::convert_matrix(const aiMatrix4x4 &aiMat) {
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
            vertex.tex_coord.y = 1.0 - ai_tex_coord.y;
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

    std::map<TextureType, Texture> texmap;

    if (!pbr) {
        aiMaterial *material = scene->mMaterials[ai_mesh->mMaterialIndex];
        std::vector<Texture> ambient_textures = load_texture(material, aiTextureType_AMBIENT, true);
        std::vector<Texture> diffuse_textures = load_texture(material, aiTextureType_DIFFUSE, true);
        std::vector<Texture> specular_textures = load_texture(material, aiTextureType_SPECULAR, true);
        std::vector<Texture> normal_textures = load_texture(material, height_normals ? aiTextureType_HEIGHT : aiTextureType_NORMALS, true);
        std::vector<Texture> height_textures = load_texture(material, aiTextureType_HEIGHT, false);

        if (ambient_textures.size()) texmap[TEXTURE_TYPE_AMBIENT_MAP] = ambient_textures[0];
        if (diffuse_textures.size()) texmap[TEXTURE_TYPE_DIFFUSE_MAP] = diffuse_textures[0];
        if (specular_textures.size()) texmap[TEXTURE_TYPE_SPECULAR_MAP] = specular_textures[0];
        if (normal_textures.size()) texmap[TEXTURE_TYPE_NORMAL_MAP] = normal_textures[0];
        if (height_textures.size()) texmap[TEXTURE_TYPE_HEIGHT_MAP] = height_textures[0];
    } else {
        std::cout << "Getting albedo" << std::endl;
        Texture albedo = load_texture_from_name("albedo.tga", true);
        Texture metallic = load_texture_from_name("metallic.tga", true);
        std::cout << "Getting normal" << std::endl;
        Texture normal = load_texture_from_name("normal.tga", false);
        std::cout << "Getting roughness" << std::endl;
        Texture roughness = load_texture_from_name("roughness.tga", false);
        std::cout << "Getting ao" << std::endl;
        Texture ao = load_texture_from_name("ao.tga", false);

        texmap[TEXTURE_TYPE_ALBEDO_MAP] = albedo; 
        texmap[TEXTURE_TYPE_METALLIC_MAP] = metallic; 
        texmap[TEXTURE_TYPE_ROUGHNESS_MAP] = roughness;
        texmap[TEXTURE_TYPE_NORMAL_MAP] = normal;
        texmap[TEXTURE_TYPE_AO_MAP] = ao;
    }

    return Mesh(
        vertex_buffer,
        vertices,
        indices,
        model * transformation,
        texmap
    );
}

Texture Model::load_texture_from_name(std::string texname, bool srgb) {
    if (loaded_textures.count(texname) > 0) {
        return loaded_textures.at(texname);
    }
    if (unit >= 16) {
        std::cout << "Can have at most 16 textures for same mesh!\n";
        exit(EXIT_FAILURE);
    }
    Texture texture(directory + "/" + texname, unit++, srgb);
    loaded_textures[texname] = texture;
    std::cout << "Got " << texname << std::endl;
    return texture;
} 

std::vector<Texture> Model::load_texture(aiMaterial *material, aiTextureType type, bool srgb) {
    std::vector<Texture> textures;
    for(uint32_t i = 0; i < material->GetTextureCount(type); i++) {

        aiString str;
        material->GetTexture(type, i, &str);

        std::string texname = std::string(str.C_Str());
        textures.push_back(load_texture_from_name(texname, srgb));
    }
    return textures;
}
