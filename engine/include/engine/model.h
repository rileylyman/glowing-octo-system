#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <fluidsim/fluidsim.h>
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/vertex.h"
#include "engine/debug.h"
#include "engine/camera.h"
#include "engine/physics.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>

//
// Determines what shader uniforms need to be passed
// to the shader so that the mesh can be drawn. For instance,
// if the mesh's shader type is PBR_TEXTURED, then we need
// to pass the BaseColor texture, the normal map, the metallic
// and roughness maps, etc. However, if the shader type is
// RAW_COLOR, then we just need to pass a color value to the
// shader.
//
enum MeshShaderType {
    PBR_SOLID = 0,
    PBR_TEXTURED,
    BP_SOLID,
    BP_TEXTURED,
    RAW_COLOR,
    RAW_TEXTURE,
};

//
// Additional options that can be passed as a bit vector
// to change the behavior of the shader. For instance,
// if there is no ambient occlusion map for this mesh,
// then we can set mesh.shader_flags |= NO_AO_MAP
//
enum MeshShaderBits {
    NO_AO_MAP = 1 << 0,
    METALLIC_ROUGHNESS_COMBINED = 1 << 1,
};

struct Model;
struct Mesh;

struct Mask {
    Mask(Texture3D tex, Model *parent, glm::mat4 bind_matrix, glm::vec3 least, glm::vec3 most) 
    : parent(parent), tex(tex), bind_matrix(bind_matrix), bbox_least(least), bbox_most(most) {}
    Texture3D tex;
    glm::mat4 bind_matrix;
    Model *parent;
    glm::vec3 bbox_most, bbox_least;
};

//
// A Mesh is the atomic unit of a model. It describes a particular set
// of vertices, as well as how they should be shaded and drawn.
//
struct Mesh {
    //
    // The shader type and shader option flags for this mesh
    //
    MeshShaderType shader_type;
    uint32_t shader_flags;

    //
    // The coordinates of the extrema of this mesh's bounding box 
    //
    glm::vec3 bbox_least, bbox_most;

    uint32_t mask_width = 20, mask_height = 20, mask_depth = 20;
    std::vector<float> mask_data;

    //
    // The constructor takes a (possibly empty) vertex
    // buffer along with an vector of vertices, and will
    // take care of filling the buffer with these vertices.
    //
    Mesh(VertexBuffer *vertex_buffer,
        std::vector<Vertex> vertices, 
        std::vector<uint32_t> indices,
        glm::mat4 bind_matrix,
        Model *parent_model,
        MeshShaderType shader_type,
        uint32_t shader_flags,
        std::map<TextureType, Texture> texmap);
    ~Mesh();
    
    //
    // Draw this mesh using the currently bound shader
    //
    void draw(ShaderProgram shader, Camera *camera);

    //
    // Draw the bounding box for this mesh
    //
    void draw_bounding_box(Camera *camera);

    //
    // Get the full local->world space transformation matrix
    //
    glm::mat4 model(); 

    //
    // Get the 3D mask texture for this mesh
    //
    Mask get_mask(uint32_t unit);

    //
    // The model transformation of the model which contains
    // this mesh.
    //
    Model *parent_model;

private:

    uint64_t hash_position(glm::vec3 position);
    uint64_t hash_func(uint32_t ix, uint32_t iy, uint32_t iz);
    void generate_mask_data(std::vector<Vertex> vertices);

    //
    // The mesh local->model local transformation
    //
    glm::mat4 bind_matrix;


    //
    // VAO, VBO, and shader program for this mesh's bounding box
    //
    GLuint bbox_vao = 0, bbox_vbo = 0;
    static ShaderProgram *bbox_shader;

    //
    // Vertex mesh information for drawing
    //
    VertexBuffer *vertex_buffer;
    uint32_t vertex_buffer_index;
    size_t indices_size;

    //
    // List of retrieved textures for this mesh
    //
    std::map<TextureType, Texture> texmap;

    // @Performance
    //
    // Optional solid materials for this mesh, if it is not textured
    //
    BlinnPhongSolidMaterial bp_solid_material;
    PBRSolidMaterial pbr_solid_material;
    
    friend struct Model;
};

//
// A model consists of one or more meshes, which are the
// atomic unit of shaded geometry. For example, a human model
// may have a separate mesh for the shirt, the eyes, etc. Each
// mesh can have different textures and a different shader, allowing
// the model to be complex.
//
struct Model {

    //
    // This constructor takes a vertex buffer to store mesh data within, the pathname
    // from which to load the model, the MeshShaderType, any shader flags, and whether
    // the height texture should be used as the normal texture.
    //
    Model(
        VertexBuffer *vertex_buffer, 
        std::string pathname, 
        MeshShaderType shader_type, 
        uint32_t shader_flags, 
        RigidBodyType type,
        glm::vec3 initial_position,
        glm::vec3 initial_rotation,
        float mass,
        bool gravity = true,
        bool height_normals=false);

    //
    // Create a model from raw vertex data and a constant Blinn-Phong material
    //
    Model(
        VertexBuffer *vertex_buffer, 
        std::vector<Vertex> vertices, 
        std::vector<uint32_t> indices, 
        BlinnPhongSolidMaterial material, 
        RigidBodyType type, 
        glm::vec3 initial_position,
        glm::vec3 initial_rotation,
        bool gravity = true
    ): physics_obj(new PhysicsObject(initial_position, initial_rotation, type, 10.0, gravity)) {
        meshes = { Mesh(vertex_buffer, vertices, indices, glm::mat4(1.0f), this, BP_SOLID, 0, {}) };
        meshes[0].bp_solid_material = material;
        gen_bbox(vertices);
    }

    //
    // Create a model from raw vertex data and a constant PBR material
    //
    Model(
        VertexBuffer *vertex_buffer, 
        std::vector<Vertex> vertices, 
        std::vector<uint32_t> indices, 
        PBRSolidMaterial material, 
        RigidBodyType type, 
        glm::vec3 initial_position,
        glm::vec3 initial_rotation,
        bool gravity = true
    ) : physics_obj(new PhysicsObject(initial_position, initial_rotation, type, 10.0, gravity)) {
        meshes = { Mesh(vertex_buffer, vertices, indices, glm::mat4(1.0f), this, PBR_SOLID, 0, {}) };
        meshes[0].pbr_solid_material = material;
        gen_bbox(vertices);
    }

    //
    // Create a model from raw vertex data and a texture
    //
    Model(
        VertexBuffer *vertex_buffer, 
        std::vector<Vertex> vertices, 
        std::vector<uint32_t> indices, 
        std::string texture, 
        RigidBodyType type, 
        glm::vec3 initial_position,
        glm::vec3 initial_rotation,
        bool gravity = true
    ) : physics_obj(new PhysicsObject(initial_position, initial_rotation, type, 10.0, gravity)) {
        directory = "";
        Texture tex = load_texture_from_name(texture, true);
        meshes = { Mesh(vertex_buffer, vertices, indices, glm::mat4(1.0f), this, RAW_TEXTURE, 0, {{TEXTURE_TYPE_DIFFUSE_MAP , tex}}) };
        gen_bbox(vertices);
    }

    //
    // Destructor
    //
    ~Model();

    //
    // Draws the model with the specified shader. Basically just calls `draw` with the 
    // arguments on every mesh within this model.
    //
    void draw(ShaderProgram shader_prog, Camera *camera);

    //
    // Draw the bounding box for this model.
    //
    void draw_bounding_box(Camera *camera);

    void pressure_force(Fluidsim::Engine &fs, int num_samples_sides, int num_side_subdivisions, glm::vec3 offset, glm::mat4 object_m) {
        // body is the reactphysics3d dynamic collision body
        // physics_obj->body->applyTorque();
        // physics_obj->body->applyForce()
        
        // coordinates of least extent of bounding box are in bbox_least
        // coordinates of greatest extent                     bbox_most

        // call this every frame

        float width   = (bbox_most.x - bbox_least.x);
        float height  = (bbox_most.y - bbox_least.y);
        float depth   = (bbox_most.z - bbox_least.z);
        glm::vec4 center_box_offset(-width/2, -height/2, -depth/2, 0.0f); // (Go from corner centered at origin to box centered at origin)

        static int index = 0;
        int next_index = (index + 1) % 3;

        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        glGetError();
        glBindBuffer(GL_PIXEL_PACK_BUFFER, Physics::instance->pbos[index]);
        glBindTexture(GL_TEXTURE_3D, fs.prescpy[index].id);
        glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, 0);

        glBindBuffer(GL_PIXEL_PACK_BUFFER, Physics::instance->pbos[next_index]);
        GLfloat *ptr = (GLfloat *)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);


        auto sample_pressure_from_box_coord = [fs, object_m, ptr, offset](glm::vec4 box_coord) {
            glm::vec4 world_coord = object_m * box_coord;
            // Sample from image based on world coords

            float dim_x = (float)fs.grid_width * fs.sclx;
            float dim_y = (float)fs.grid_height * fs.scly;
            float dim_z = (float)fs.grid_depth * fs.sclz;

            // float float_cell_x = ((world_coord.x + dim_x / 2.0) / dim_x) * (float)fs.grid_width;
            // float float_cell_y = ((world_coord.y + dim_y / 2.0) / dim_y) * (float)fs.grid_height;
            // float float_cell_z = ((world_coord.z + dim_z / 2.0) / dim_z) * (float)fs.grid_depth;

            world_coord -= glm::vec4(offset, 0.0f);
            world_coord /= glm::vec4(dim_x, dim_y, dim_z, 1.0f);
            world_coord += glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
            world_coord *= glm::vec4((float) fs.grid_width, (float) fs.grid_height, (float) fs.grid_depth, 1.0f);

            float float_cell_x = world_coord.x;
            float float_cell_y = world_coord.y;
            float float_cell_z = world_coord.z;
            
            if (float_cell_x >= (float)fs.grid_width) {
                return 0.0f;
            } else if (float_cell_x < 0.0) {
                return 0.0f;
            }
            if (float_cell_y >= (float)fs.grid_height) {
                return 0.0f;
            } else if (float_cell_y < 0.0) {
                return 0.0f;
            }
            if (float_cell_z >= (float)fs.grid_depth) {
                return 0.0f;
            } else if (float_cell_z < 0.0) {
                return 0.0f;
            }
            
            uint32_t cell_x = (uint32_t)float_cell_x;
            uint32_t cell_y = (uint32_t)float_cell_y;
            uint32_t cell_z = (uint32_t)float_cell_z;

            float pressure = 0.0f;

            if (ptr) {
                // process pixels
                auto get_pressure = [fs, ptr] (uint32_t w, uint32_t h, uint32_t d) {
                    uint32_t pixel_index = 4 * (w + fs.grid_width * d + fs.grid_width * fs.grid_depth * h);
                    return ptr[pixel_index];
                };
                pressure = get_pressure(cell_x, cell_y, cell_z);

                //std::cout << "Pressure at ( " << cell_x << ", " << cell_y << ", " << cell_z << "): " << pressure << std::endl;
                //for (int i = 0; i < 10; i++) {
                //    std::cout << ptr[i] << ", ";
                //}
                //std::cout << ") " << std::endl;
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            }

            return pressure;
        };

        // Induce a small perturbation to surface to get nonzero pressure values
        float dv = 1.0001f * sqrt(fs.sclx * fs.sclx + fs.scly * fs.scly + fs.sclz * fs.sclz);
    
        //
        // SAMPLE ON LEFT SIDE OF THE BOX (y,z varies)
        //

        for (int i = 0; i < num_side_subdivisions; i++) {
            for (int j = 0; j < num_side_subdivisions; j++) {
                // Calculate and apply force the subdivided section of the box
                glm::vec4 force(0.0f, 0.0f, 0.0f, 1.0f);

                for (int k = 0; k < num_samples_sides; k++) {
                    // Get surface normal of the box
                    glm::vec4 norm(-1.0f, 0.0f, 0.0f, 0.0f);

                    // Get random point on the side
                    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    float s = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    
                    glm::vec4 sample_loc(
                        0.f - dv,
                        (r * (height / (float) num_side_subdivisions)) + ((float) i * (height / num_side_subdivisions)),
                        (s * (depth / (float) num_side_subdivisions)) + ((float) j * (depth / num_side_subdivisions)),
                        1.0f
                    );

                    sample_loc += center_box_offset;

                    // Get the probability of the left side of the box
                    float prob = 1 / (depth * height / ((float)num_side_subdivisions * (float)num_side_subdivisions));
                    
                    // Get pressure
                    float pressure = sample_pressure_from_box_coord(sample_loc);
                    std::cout << "LEFT::" << pressure << std::endl;

                    // Get directional pressure
                    glm::vec4 p = pressure * object_m * norm;

                    // Integrate small force
                    force += p / prob;
                }
                
                force /= (float)num_samples_sides;
                    
                glm::vec4 location2applyForce(
                    0.0f,
                    (0.5f * (height / (float) num_side_subdivisions)) + ((float) i * (height / num_side_subdivisions)),
                    (0.5f * (depth / (float) num_side_subdivisions)) + ((float) j * (depth / num_side_subdivisions)),
                    1.0f
                );

                location2applyForce += center_box_offset;

                // TODO: APPLY INTEGRATED FORCE
                physics_obj->apply_force_to_point(force, location2applyForce);
            }
        }

        //
        // SAMPLE ON RIGHT SIDE OF THE BOX (y,z varies)
        //

        for (int i = 0; i < num_side_subdivisions; i++) {
            for (int j = 0; j < num_side_subdivisions; j++) {
                // Calculate and apply force the subdivided section of the box
                glm::vec4 force(0.0f, 0.0f, 0.0f, 1.0f);

                for (int k = 0; k < num_samples_sides; k++) {
                    // Get surface normal of the box
                    glm::vec4 norm(1.0f, 0.0f, 0.0f, 0.0f);

                    // Get random point on the side
                    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    float s = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    
                    glm::vec4 sample_loc(
                        width + dv,
                        (r * (height / (float) num_side_subdivisions)) + ((float) i * (height / num_side_subdivisions)),
                        (s * (depth / (float) num_side_subdivisions)) + ((float) j * (depth / num_side_subdivisions)),
                        1.0f
                    );

                    sample_loc += center_box_offset;

                    // Get the probability of the left side of the box
                    float prob = 1 / (depth * height / ((float)num_side_subdivisions * (float)num_side_subdivisions));
                    
                    // Get pressure
                    float pressure = sample_pressure_from_box_coord(sample_loc);
                    std::cout << "RIGHT::"  << pressure << std::endl;

                    // Get directional pressure
                    glm::vec4 p = pressure * object_m * norm;

                    // Integrate small force
                    force += p / prob;
                }
                
                force /= (float)num_samples_sides;
                    
                glm::vec4 location2applyForce(
                    width,
                    (0.5f * (height / (float) num_side_subdivisions)) + ((float) i * (height / num_side_subdivisions)),
                    (0.5f * (depth / (float) num_side_subdivisions)) + ((float) j * (depth / num_side_subdivisions)),
                    1.0f
                );

                location2applyForce += center_box_offset;

                // TODO: APPLY INTEGRATED FORCE
                physics_obj->apply_force_to_point(force, location2applyForce);
            }
        }


        //
        // SAMPLE ON BOTTOM SIDE OF THE BOX (x,z varies)
        //

        for (int i = 0; i < num_side_subdivisions; i++) {
            for (int j = 0; j < num_side_subdivisions; j++) {
                // Calculate and apply force the subdivided section of the box
                glm::vec4 force(0.0f, 0.0f, 0.0f, 1.0f);

                for (int k = 0; k < num_samples_sides; k++) {
                    // Get surface normal of the box
                    glm::vec4 norm(0.0f, -1.0f, 0.0f, 0.0f);

                    // Get random point on the side
                    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    float s = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    
                    glm::vec4 sample_loc(
                        (r * (width / (float) num_side_subdivisions)) + ((float) i * (width / num_side_subdivisions)),
                        0.0f - dv,
                        (s * (depth / (float) num_side_subdivisions)) + ((float) j * (depth / num_side_subdivisions)),
                        1.0f
                    );

                    sample_loc += center_box_offset;

                    // Get the probability of the left side of the box
                    float prob = 1 / (depth * width / ((float)num_side_subdivisions * (float)num_side_subdivisions));
                    
                    // Get pressure
                    float pressure = sample_pressure_from_box_coord(sample_loc);
                    std::cout << "BOT::"  << pressure << std::endl;

                    // Get directional pressure
                    glm::vec4 p = pressure * object_m * norm;

                    // Integrate small force
                    force += p / prob;
                }
                
                force /= (float)num_samples_sides;
                    
                glm::vec4 location2applyForce(
                    (0.5f * (width / (float) num_side_subdivisions)) + ((float) i * (width / num_side_subdivisions)),
                    0.0f,
                    (0.5f * (depth / (float) num_side_subdivisions)) + ((float) j * (depth / num_side_subdivisions)),
                    1.0f
                );

                location2applyForce += center_box_offset;

                // TODO: APPLY INTEGRATED FORCE
                physics_obj->apply_force_to_point(force, location2applyForce);
            }
        }


        //
        // SAMPLE ON TOP SIDE OF THE BOX (x,z varies)
        //

        for (int i = 0; i < num_side_subdivisions; i++) {
            for (int j = 0; j < num_side_subdivisions; j++) {
                // Calculate and apply force the subdivided section of the box
                glm::vec4 force(0.0f, 0.0f, 0.0f, 1.0f);

                for (int k = 0; k < num_samples_sides; k++) {
                    // Get surface normal of the box
                    glm::vec4 norm(0.0f, 1.0f, 0.0f, 0.0f);

                    // Get random point on the side
                    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    float s = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    
                    glm::vec4 sample_loc(
                        (r * (width / (float) num_side_subdivisions)) + ((float) i * (width / num_side_subdivisions)),
                        height + dv,
                        (s * (depth / (float) num_side_subdivisions)) + ((float) j * (depth / num_side_subdivisions)),
                        1.0f
                    );

                    sample_loc += center_box_offset;

                    // Get the probability of the left side of the box
                    float prob = 1 / (depth * width / ((float)num_side_subdivisions * (float)num_side_subdivisions));
                    
                    // Get pressure
                    float pressure = sample_pressure_from_box_coord(sample_loc);
                    std::cout << "TOP::"  << pressure << std::endl;

                    // Get directional pressure
                    glm::vec4 p = pressure * object_m * norm;

                    // Integrate small force
                    force += p / prob;
                }
                
                force /= (float)num_samples_sides;
                    
                glm::vec4 location2applyForce(
                    (0.5f * (width / (float) num_side_subdivisions)) + ((float) i * (width / num_side_subdivisions)),
                    height,
                    (0.5f * (depth / (float) num_side_subdivisions)) + ((float) j * (depth / num_side_subdivisions)),
                    1.0f
                );

                location2applyForce += center_box_offset;

                // TODO: APPLY INTEGRATED FORCE
                physics_obj->apply_force_to_point(force, location2applyForce);
            }
        }


        //
        // SAMPLE ON DOWN SIDE OF THE BOX (x, y varies)
        //

        for (int i = 0; i < num_side_subdivisions; i++) {
            for (int j = 0; j < num_side_subdivisions; j++) {
                // Calculate and apply force the subdivided section of the box
                glm::vec4 force(0.0f, 0.0f, 0.0f, 1.0f);

                for (int k = 0; k < num_samples_sides; k++) {
                    // Get surface normal of the box
                    glm::vec4 norm(0.0f, 0.0f, -1.0f, 0.0f);

                    // Get random point on the side
                    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    float s = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    
                    glm::vec4 sample_loc(
                        (r * (width / (float) num_side_subdivisions)) + ((float) i * (width / num_side_subdivisions)),
                        (s * (height / (float) num_side_subdivisions)) + ((float) j * (height / num_side_subdivisions)),
                        0.0f - dv,
                        1.0f
                    );

                    sample_loc += center_box_offset;

                    // Get the probability of the left side of the box
                    float prob = 1 / (height * width / ((float)num_side_subdivisions * (float)num_side_subdivisions));
                    
                    // Get pressure
                    float pressure = sample_pressure_from_box_coord(sample_loc);
                    std::cout << "DOWN::"  << pressure << std::endl;

                    // Get directional pressure
                    glm::vec4 p = pressure * object_m * norm;

                    // Integrate small force
                    force += p / prob;
                }
                
                force /= (float)num_samples_sides;
                    
                glm::vec4 location2applyForce(
                    (0.5f * (width / (float) num_side_subdivisions)) + ((float) i * (width / num_side_subdivisions)),
                    (0.5f * (height / (float) num_side_subdivisions)) + ((float) j * (height / num_side_subdivisions)),
                    0.0f,
                    1.0f
                );

                location2applyForce += center_box_offset;

                // TODO: APPLY INTEGRATED 
                physics_obj->apply_force_to_point(force, location2applyForce);
            }
        }

        //
        // SAMPLE ON UP SIDE OF THE BOX (x, y varies)
        //

        for (int i = 0; i < num_side_subdivisions; i++) {
            for (int j = 0; j < num_side_subdivisions; j++) {
                // Calculate and apply force the subdivided section of the box
                glm::vec4 force(0.0f, 0.0f, 0.0f, 1.0f);

                for (int k = 0; k < num_samples_sides; k++) {
                    // Get surface normal of the box
                    glm::vec4 norm(0.0f, 0.0f, 1.0f, 0.0f);

                    // Get random point on the side
                    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    float s = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    
                    glm::vec4 sample_loc(
                        (r * (width / (float) num_side_subdivisions)) + ((float) i * (width / num_side_subdivisions)),
                        (s * (height / (float) num_side_subdivisions)) + ((float) j * (height / num_side_subdivisions)),
                        depth + dv,
                        1.0f
                    );

                    sample_loc += center_box_offset;

                    // Get the probability of the left side of the box
                    float prob = 1 / (height * width / ((float)num_side_subdivisions * (float)num_side_subdivisions));
                    
                    // Get pressure
                    float pressure = sample_pressure_from_box_coord(sample_loc);
                    std::cout << "UP::"  << pressure << std::endl;

                    // Get directional pressure
                    glm::vec4 p = pressure * object_m * norm;

                    // Integrate small force
                    force += p / prob;
                }
                
                force /= (float)num_samples_sides;
                    
                glm::vec4 location2applyForce(
                    (0.5f * (width / (float) num_side_subdivisions)) + ((float) i * (width / num_side_subdivisions)),
                    (0.5f * (height / (float) num_side_subdivisions)) + ((float) j * (height / num_side_subdivisions)),
                    depth,
                    1.0f
                );

                location2applyForce += center_box_offset;

                // TODO: APPLY INTEGRATED FORCE
                physics_obj->apply_force_to_point(force, location2applyForce);
            }
        }


        // CLEAN UP PIXEL BUFFER
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glGetError();
        index = next_index;


    }

    //
    // The local->world space transform for this model
    //
    glm::mat4 model() {
        return physics_obj->get_model_matrix() * inverse_bbox_center_transform;
    }

    inline std::vector<Mesh> get_meshes() { return meshes; }


    PhysicsObject *physics_obj;
private:
    std::string name;

    //
    // The list of meshes contained within this model.
    //
    std::vector<Mesh> meshes;

    //
    // Information needed for drawing the model's bounding box
    //
    GLuint bbox_vao = 0, bbox_vbo = 0;
    glm::vec3 bbox_least, bbox_most;
    static ShaderProgram *bbox_shader;


    glm::mat4 inverse_bbox_center_transform = glm::mat4(-1.0);

    //
    // Vertex buffer which stores all of the data for all of
    // this models sub-meshes
    //
    VertexBuffer *vertex_buffer;

    //
    // The directory this model is loaded from. Used to find textures
    // and other assets at load-time
    //
    std::string directory;

    //
    // A static map from texture names to loaded textures. Avoids
    // loading of the same texture more than once
    //
    static std::map<std::string, Texture> loaded_textures;

    //
    // The current texture unit for this model.
    //
    uint32_t unit = 0;

    //
    // Generate the bounding box extrema based on
    // vertex data
    //
    void gen_bbox(std::vector<Vertex> vertices);

    //
    // Utility function to convert an Assimp matrix to a
    // GLM matrix
    //
    glm::mat4 convert_matrix(const aiMatrix4x4 &aiMat); 

    //
    // Functions used to load the mesh from a file. Makes heavy use of
    // the Assimp model loading library.
    //
    void load_model(std::string pathname, MeshShaderType shader_type, uint32_t shader_flags, bool height_normals);
    Mesh process_mesh(aiMesh *ai_mesh, const aiScene *scene, glm::mat4 transformation, MeshShaderType shader_type, uint32_t shader_flags, bool height_normals); 
    void process_node(aiNode *node, const aiScene* scene, glm::mat4, MeshShaderType shader_type, uint32_t shader_flags, bool height_normals);
    Texture load_texture_from_name(std::string texname, bool srgb);
    std::vector<Texture> load_texture(aiMaterial *material, aiTextureType type, bool srgb);
};

void draw_bounding_box_general(glm::vec3 bbox_least, glm::vec3 bbox_most, GLuint vao, GLuint vbo, ShaderProgram *bbox_shader, glm::mat4 model, Camera *camera);