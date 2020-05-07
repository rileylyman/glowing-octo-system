#include <glm/gtc/matrix_transform.hpp>
#include <assimp/pbrmaterial.h>
#include <iostream>
#include <set>
#include <tgmath.h>
#include "engine/model.h"
#include "engine/imgui-instance.h"

std::map<std::string, Texture> Model::loaded_textures = {};

ShaderProgram *Mesh::bbox_shader = nullptr;
ShaderProgram *Model::bbox_shader = nullptr;

Mesh::Mesh(
    VertexBuffer *vertex_buffer,
    std::vector<Vertex> vertices, 
    std::vector<uint32_t> indices,
    glm::mat4 bind_matrix,
    Model *parent_model,
    MeshShaderType shader_type,
    uint32_t shader_bits,
    std::map<TextureType, Texture> texmap)
: vertex_buffer(vertex_buffer), indices_size(indices.size()), bind_matrix(bind_matrix), parent_model(parent_model), texmap(texmap), shader_type(shader_type), shader_flags(shader_bits)
{

    if (bbox_shader == nullptr) {
        bbox_shader = new ShaderProgram("src/shaders/bbox.vert", "src/shaders/bbox.frag");
    }
    bbox_least = glm::vec3(vertices[0].position);
    bbox_most  = glm::vec3(vertices[0].position);
    bbox_least = glm::vec3(bind_matrix * glm::vec4(bbox_least.x, bbox_least.y, bbox_least.z, 1.0f));
    bbox_most = glm::vec3(bind_matrix * glm::vec4(bbox_most.x, bbox_most.y, bbox_most.z, 1.0f));

    for (Vertex vert : vertices) {
        glm::vec4 position = bind_matrix * glm::vec4(vert.position.x, vert.position.y, vert.position.z, 1.0f);
        bbox_least.x = std::min(position.x, bbox_least.x);
        bbox_least.y = std::min(position.y, bbox_least.y);
        bbox_least.z = std::min(position.z, bbox_least.z);

        bbox_most.x = std::max(position.x, bbox_most.x);
        bbox_most.y = std::max(position.y, bbox_most.y);
        bbox_most.z = std::max(position.z, bbox_most.z);
    }

    generate_mask_data(vertices);

    vertex_buffer_index = vertex_buffer->add_data(vertices, indices);
}


uint64_t Mesh::hash_position(glm::vec3 position) {
    float x = (position.x - bbox_least.x) / (bbox_most.x - bbox_least.x) * (float) mask_width;
    float y = (position.y - bbox_least.y) / (bbox_most.y - bbox_least.y) * (float) mask_height;
    float z = (position.z - bbox_least.z) / (bbox_most.z - bbox_least.z) * (float) mask_depth;

    uint32_t ix = static_cast<uint32_t> (std::floor(x));
    uint32_t iy = static_cast<uint32_t> (std::floor(y));
    uint32_t iz = static_cast<uint32_t> (std::floor(z));

    return hash_func(ix, iy, iz);
}

uint64_t Mesh::hash_func(uint32_t ix, uint32_t iy, uint32_t iz) {
    uint64_t hash = iz << 32 + iy << 16 + ix;
    return hash * 2654435761 >> 16;
}

void Mesh::generate_mask_data(std::vector<Vertex> vertices) {

    std::set<uint64_t> spatial_hm;

    for (Vertex &vert: vertices) {
        float pos_hash = hash_position(vert.position);
        spatial_hm.insert(pos_hash);
    }

    mask_data.reserve(4 * mask_width * mask_height * mask_depth);
    for (uint32_t h = 0; h < mask_height; h++) {
        for (uint32_t d = 0; d < mask_depth; d++) {
            for (uint32_t w = 0; w < mask_width; w++) {

                uint64_t hash = hash_func(w, h, d);
                if (spatial_hm.count(hash) > 0) {
                    mask_data.push_back(1.0f);
                    mask_data.push_back(0.0f);
                    mask_data.push_back(0.0f);
                    mask_data.push_back(1.0f);
                }  else {
                    mask_data.push_back(0.0f);
                    mask_data.push_back(0.0f);
                    mask_data.push_back(0.0f);
                    mask_data.push_back(1.0f);
                }
            }
        }
    }
}

Mask Mesh::get_mask(uint32_t unit) {
    return Mask(Texture3D(mask_width, mask_height, mask_depth, unit, mask_data), parent_model, bind_matrix, bbox_least, bbox_most);
}

glm::mat4 Mesh::model() {
    return parent_model->model() * bind_matrix;
}

Mesh::~Mesh() {
    //delete bbox_shader;
    //glDeleteVertexArrays(1, &bbox_vao);
    //glDeleteBuffers(1, &bbox_vbo);
}

Model::~Model() {
    //delete bbox_shader;
    //glDeleteVertexArrays(1, &bbox_vao);
    //glDeleteBuffers(1, &bbox_vbo);
}

void Mesh::draw(ShaderProgram shader, Camera *camera) {

    shader.use();
    switch (shader_type) {
        case PBR_TEXTURED:
            shader.setVec3("camera_pos", camera->position);
            shader.setMat4("model", model());
            shader.setMat3("normal_matrix", glm::transpose(glm::inverse(glm::mat3(model()))));
            shader.setMat4("transform", camera->projection() * camera->view() * model());
            if (shader_flags & METALLIC_ROUGHNESS_COMBINED) {
                texmap[TEXTURE_TYPE_METALLIC_ROUGHNESS_MAP].use();
                shader.setInt("u_Material.metallicRoughness", texmap[TEXTURE_TYPE_METALLIC_ROUGHNESS_MAP].unit);
                shader.setBool("u_MetallicRoughnessCombined", true);
            } else {
                texmap[TEXTURE_TYPE_METALLIC_MAP].use();
                texmap[TEXTURE_TYPE_ROUGHNESS_MAP].use();
                shader.setInt("u_Material.metallic", texmap[TEXTURE_TYPE_METALLIC_MAP].unit);
                shader.setInt("u_Material.roughness", texmap[TEXTURE_TYPE_ROUGHNESS_MAP].unit);
                shader.setBool("u_MetallicRoughnessCombined", false);
            }
            texmap[TEXTURE_TYPE_ALBEDO_MAP].use();
            texmap[TEXTURE_TYPE_AO_MAP].use();
            texmap[TEXTURE_TYPE_NORMAL_MAP].use();
            shader.setInt("u_Material.albedo", texmap[TEXTURE_TYPE_ALBEDO_MAP].unit);
            shader.setInt("u_Material.ao", texmap[TEXTURE_TYPE_AO_MAP].unit);
            shader.setInt("u_Material.normal", texmap[TEXTURE_TYPE_NORMAL_MAP].unit);
            shader.setBool("u_Solid", false);
        break;
        case PBR_SOLID:
            shader.setVec3("camera_pos", camera->position);
            shader.setMat4("model", model());
            shader.setMat3("normal_matrix", glm::transpose(glm::inverse(glm::mat3(model()))));
            shader.setMat4("transform", camera->projection() * camera->view() * model());
            shader.setFloat("u_SolidMaterial.metallic", pbr_solid_material.metallic);
            shader.setFloat("u_SolidMaterial.roughness", pbr_solid_material.roughness);
            shader.setVec3("u_SolidMaterial.albedo", pbr_solid_material.albedo);
            shader.setBool("u_Solid", true);
        break;
        case BP_TEXTURED:
            shader.setVec3("camera_pos", camera->position);
            shader.setMat4("model", model());
            shader.setMat3("normal_matrix", glm::transpose(glm::inverse(glm::mat3(model()))));
            shader.setMat4("transform", camera->projection() * camera->view() * model());
            texmap[TEXTURE_TYPE_AMBIENT_MAP].use();
            texmap[TEXTURE_TYPE_DIFFUSE_MAP].use();
            texmap[TEXTURE_TYPE_SPECULAR_MAP].use();
            texmap[TEXTURE_TYPE_NORMAL_MAP].use();
            texmap[TEXTURE_TYPE_HEIGHT_MAP].use();
            shader.setInt("material.ambient", texmap[TEXTURE_TYPE_AMBIENT_MAP].unit);
            shader.setInt("material.diffuse", texmap[TEXTURE_TYPE_DIFFUSE_MAP].unit);
            shader.setInt("material.specular", texmap[TEXTURE_TYPE_SPECULAR_MAP].unit);
            shader.setInt("material.normal", texmap[TEXTURE_TYPE_NORMAL_MAP].unit);
            shader.setInt("material.height", texmap[TEXTURE_TYPE_HEIGHT_MAP].unit);
            shader.setFloat("material.shininess", 64.0f);
            shader.setBool("u_RenderNormals", ImGuiInstance::render_normals);
            shader.setBool("u_Solid", false);
        break;
        case BP_SOLID:
            shader.setVec3("camera_pos", camera->position);
            shader.setMat4("model", model());
            shader.setMat3("normal_matrix", glm::transpose(glm::inverse(glm::mat3(model()))));
            shader.setMat4("transform", camera->projection() * camera->view() * model());
            shader.setVec3("u_SolidMaterial.ambient",  bp_solid_material.ambient);
            shader.setVec3("u_SolideMaterial.diffuse",  bp_solid_material.diffuse);
            shader.setVec3("u_SolidMaterial.specular", bp_solid_material.specular);
            shader.setFloat("u_SolidMaterial.shininess", 64.0f);
            shader.setBool("u_RenderNormals", false);
            shader.setBool("u_Solid", true);
        break;
        case RAW_COLOR:
        break;
        case RAW_TEXTURE:
        break;
    }
    vertex_buffer->draw(vertex_buffer_index, indices_size);
}

Model::Model(
    VertexBuffer *vertex_buffer, 
    std::string pathname, 
    MeshShaderType shader_type, 
    uint32_t shader_flags, 
    RigidBodyType type,
    glm::vec3 initial_position,
    glm::vec3 initial_rotation,
    float mass, 
    bool gravity,
    bool height_normals)
: vertex_buffer(vertex_buffer) 
{
    if (bbox_shader == nullptr) {
        bbox_shader = new ShaderProgram("src/shaders/bbox.vert", "src/shaders/bbox.frag");
    }
    bbox_least = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    bbox_most = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    load_model(pathname, shader_type, shader_flags, height_normals);

    glm::vec3 half_extents;
    half_extents.x = (bbox_most.x - bbox_least.x) / 2.0;
    half_extents.y = (bbox_most.y - bbox_least.y) / 2.0;
    half_extents.z = (bbox_most.z - bbox_least.z) / 2.0;
    physics_obj = new PhysicsObject(initial_position, initial_rotation, type, mass, gravity, half_extents, bbox_least);

    glm::vec3 bbox_center = -(bbox_most + bbox_least) / 2.0f;
    inverse_bbox_center_transform = glm::translate(glm::mat4(1.0), bbox_center);
}

void Model::draw(ShaderProgram shader_prog, Camera *camera) {
    for (Mesh mesh : meshes) {
        mesh.parent_model = this;
        mesh.draw(shader_prog, camera);
    }
    if (ImGuiInstance::draw_mesh_bb) {
        for (Mesh mesh: meshes) {
            mesh.parent_model = this;
            mesh.draw_bounding_box(camera);
        }
    }
    if (ImGuiInstance::draw_model_bb) {
        draw_bounding_box(camera);
    }
}

void Model::gen_bbox(std::vector<Vertex> verts) {
    bbox_least = glm::vec3(verts[0].position);
    bbox_most  = glm::vec3(verts[0].position);
    bbox_least = glm::vec3(model() * glm::vec4(bbox_least.x, bbox_least.y, bbox_least.z, 1.0f));
    bbox_most = glm::vec3(model() * glm::vec4(bbox_most.x, bbox_most.y, bbox_most.z, 1.0f));

    for (Vertex vert : verts) {
        glm::vec4 position = model() * glm::vec4(vert.position.x, vert.position.y, vert.position.z, 1.0f);
        bbox_least.x = std::min(position.x, bbox_least.x);
        bbox_least.y = std::min(position.y, bbox_least.y);
        bbox_least.z = std::min(position.z, bbox_least.z);

        bbox_most.x = std::max(position.x, bbox_most.x);
        bbox_most.y = std::max(position.y, bbox_most.y);
        bbox_most.z = std::max(position.z, bbox_most.z);
    }

}

void draw_bounding_box_general(glm::vec3 bbox_least, glm::vec3 bbox_most, GLuint vao, GLuint vbo, ShaderProgram *bbox_shader, glm::mat4 model, Camera *camera) {

    float lx = bbox_least.x, ly = bbox_least.y, lz = bbox_least.z;
    float mx = bbox_most.x, my = bbox_most.y, mz = bbox_most.z;

    // @Performance
    float vertices[] = {
        lx, ly, lz, lx, my, lz, mx, ly, lz,
        lx, my, lz, mx, ly, lz, mx, my, lz,

        lx, ly, mz, lx, my, mz, mx, ly, mz,
        lx, my, mz, mx, ly, mz, mx, my, mz,

        mx, ly, lz, mx, my, lz, mx, ly, mz,
        mx, my, lz, mx, ly, mz, mx, my, mz,

        lx, ly, lz, lx, my, lz, lx, ly, mz,
        lx, my, lz, lx, ly, mz, lx, my, mz,

        lx, my, lz, lx, my, mz, mx, my, lz,
        lx, my, mz, mx, my, lz, mx, my, mz,

        lx, ly, lz, lx, ly, mz, mx, ly, lz,
        lx, ly, mz, mx, ly, lz, mx, ly, mz,

    };

    if (vao == 0 || vbo == 0) {
        glGenBuffers(1, &vbo);
        glGenVertexArrays(1, &vao);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(GL_FLOAT), (void *) 0);
    }
    glBindVertexArray(vao);

    bbox_shader->use();
    bbox_shader->setMat4("u_Transform", camera->projection() * camera->view() * model);

    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(float));
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (ImGuiInstance::cull_back_face) {
        glEnable(GL_CULL_FACE);
    }
}

void Mesh::draw_bounding_box(Camera *camera) {
    draw_bounding_box_general(bbox_least, bbox_most, bbox_vao, bbox_vbo, bbox_shader, model(), camera);
}

void Model::draw_bounding_box(Camera *camera) {
    draw_bounding_box_general(bbox_least, bbox_most, bbox_vao, bbox_vbo, bbox_shader, model(), camera);
}


//                                                                                                    //
// ---------------------------------------------------------------------------------------------------//
//                                  Model loading                                                     //
// ---------------------------------------------------------------------------------------------------//
//                                                                                                    //

//
// Assimp -> GLM matrix conversion
//
glm::mat4 Model::convert_matrix(const aiMatrix4x4 &aiMat) {
    return {
        aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
        aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
        aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
        aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
    };
}

//
// Load model from the specified pathname
//
void Model::load_model(std::string pathname, MeshShaderType shader_type, uint32_t shader_flags, bool height_normals) {
    
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(pathname, aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "Assimp importer error: " << importer.GetErrorString() << std::endl;
        exit(EXIT_FAILURE);
    }
    directory = pathname.substr(0, pathname.find_last_of('/'));

    process_node(scene->mRootNode, scene, glm::mat4(1.0f), shader_type, shader_flags, height_normals);
}

//
// Process all the meshes contained within the node, then process the children nodes
//
void Model::process_node(aiNode *node, const aiScene *scene, glm::mat4 transformation, MeshShaderType shader_type, uint32_t shader_flags, bool height_normals) {

    glm::mat4 current_tr = convert_matrix(node->mTransformation);
    transformation = transformation * current_tr  ;

    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh *ai_mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh mesh = process_mesh(ai_mesh, scene, transformation, shader_type, shader_flags, height_normals);

        bbox_least.x = std::min(mesh.bbox_least.x, bbox_least.x);
        bbox_least.y = std::min(mesh.bbox_least.y, bbox_least.y);
        bbox_least.z = std::min(mesh.bbox_least.z, bbox_least.z);

        bbox_most.x = std::max(mesh.bbox_most.x, bbox_most.x);
        bbox_most.y = std::max(mesh.bbox_most.y, bbox_most.y);
        bbox_most.z = std::max(mesh.bbox_most.z, bbox_most.z);

        meshes.push_back(mesh);
    }
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene, transformation, shader_type, shader_flags, height_normals);
    }
}

//
// Load an Assimp mesh into our mesh representation
//
Mesh Model::process_mesh(aiMesh *ai_mesh, const aiScene *scene, glm::mat4 transformation, MeshShaderType shader_type, uint32_t shader_flags, bool height_normals) {

    unit = 0;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i < ai_mesh->mNumVertices; i++) {
        Vertex vertex;

        aiVector3D ai_position = ai_mesh->mVertices[i];
        aiVector3D ai_normal = ai_mesh->mNormals[i];
        if (ai_mesh->mTangents && ai_mesh->mBitangents) {
            aiVector3D ai_tangent = ai_mesh->mTangents[i];
            aiVector3D ai_bitangent = ai_mesh->mBitangents[i];
            vertex.tangent = glm::vec3(ai_tangent.x, ai_tangent.y, ai_tangent.z);
            vertex.bitangent = glm::vec3(ai_bitangent.x, ai_bitangent.y, ai_bitangent.z);
        }
        else {
            vertex.tangent = glm::vec3(0.0);
            vertex.bitangent = glm::vec3(0.0);
        }
        
        vertex.position = glm::vec3(ai_position.x, ai_position.y, ai_position.z);
        vertex.normal = glm::vec3(ai_normal.x, ai_normal.y, ai_normal.z);
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

    aiMaterial *material = scene->mMaterials[ai_mesh->mMaterialIndex];
    for (int i = 1; i <= 18; i++) {
        aiString str;
        material->GetTexture((aiTextureType)i, 0, &str);
        //std::cout << i << ": " << str.C_Str() << std::endl;
    }

    if (shader_type == BP_TEXTURED) {
        std::vector<Texture> ambient_textures  = load_texture( material, aiTextureType_AMBIENT, true );
        std::vector<Texture> diffuse_textures  = load_texture( material, aiTextureType_DIFFUSE, true );
        std::vector<Texture> specular_textures = load_texture( material, aiTextureType_SPECULAR, true );
        std::vector<Texture> normal_textures   = load_texture( material, height_normals ? aiTextureType_HEIGHT : aiTextureType_NORMALS, true );
        std::vector<Texture> height_textures   = load_texture( material, aiTextureType_HEIGHT, false );

        if ( ambient_textures.size()  ) texmap[TEXTURE_TYPE_AMBIENT_MAP]  =  ambient_textures[0];
        if ( diffuse_textures.size()  ) texmap[TEXTURE_TYPE_DIFFUSE_MAP]  =  diffuse_textures[0];
        if ( specular_textures.size() ) texmap[TEXTURE_TYPE_SPECULAR_MAP] = specular_textures[0];
        if ( normal_textures.size()   ) texmap[TEXTURE_TYPE_NORMAL_MAP]   =   normal_textures[0];
        if ( height_textures.size()   ) texmap[TEXTURE_TYPE_HEIGHT_MAP]   =   height_textures[0];
    } else if (shader_type == PBR_TEXTURED) {

        // Find the albedo texture
        //aiString albedo_path;
        //aiTextureType possible_albedos[] = { aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR };

        //aiString albedo_path;
        //aiTextureType possible_albedos[] = { aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR };


        Texture albedo    = load_texture_from_name("albedo.png", true);
        Texture metallic  = load_texture_from_name("metallic.png", true);
        Texture normal    = load_texture_from_name("normal.png", false);
        Texture roughness = load_texture_from_name("roughness.png", false);
        Texture ao        = load_texture_from_name("ao.png", false);

        texmap[TEXTURE_TYPE_ALBEDO_MAP]    = albedo; 
        texmap[TEXTURE_TYPE_METALLIC_MAP]  = metallic; 
        texmap[TEXTURE_TYPE_ROUGHNESS_MAP] = roughness;
        texmap[TEXTURE_TYPE_NORMAL_MAP]    = normal;
        texmap[TEXTURE_TYPE_AO_MAP]        = ao;
    } else {
        throw "unimplemented";
    }

    return Mesh(
        vertex_buffer,
        vertices,
        indices,
        transformation,
        this,
        shader_type,
        shader_flags,
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
