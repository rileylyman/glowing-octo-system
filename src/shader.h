#pragma once

#include <glad/glad.h>
#include <stdint.h>
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include "light.h"
#include "texture.h"
#include "camera.h"

//
// Material definitions used by our shaders
//

struct BlinnPhongTexturedMaterial {
    Texture ambient, diffuse, specular, normal, height;
    float shininess;
};

struct BlinnPhongSolidMaterial {
    glm::vec3 ambient, diffuse, specular;
    float shininess;
};

struct PBRTexturedMaterial {
    Texture albedo, normal, metallic, roughness, ao;
};

struct PBRSolidMaterial {
    glm::vec3 albedo;
    float metallic, roughness;
};

//
// Represents the final, linked shader program that is used by the
// graphics pipeline to render images
//
struct ShaderProgram {

    //
    // Specify the paths for the vertex, fragment, and, optionally, the geometry shaders
    // to create a shader.
    //
    ShaderProgram(const char* vertex_path, const char* frag_path, const char* geo_path=nullptr); 

    //
    // Set the OpenGl state machine's active shader to this one, meaning that
    // all draw calls will now invoke this shader.
    //
    void use();

    //
    // Convinience function to pass in all the lighting data to the shader
    //
    void bind_lights(std::vector<DirLight *> dir_lights, std::vector<PointLight *> point_lights, std::vector<Spotlight *> spotlights);

    //
    // Functions to set uniforms within a shader by the uniform's name
    //
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec2(const std::string &name, float x, float y) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w);
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;


private:
    //
    // The id that OpenGL uses to refer to this shader.
    //
    uint32_t id;

    //
    // Caps on the number of lights you can pass into a shader
    //
    const int max_nr_dir_lights = 8;
    const int max_nr_spotlights = 1;
    const int max_nr_point_lights = 16;

    //
    // Helper functions to check for compilation and link errors
    // of the supplied shaders
    //
    static void check_link_errors(uint32_t id);
    static void check_compile_errors(uint32_t id);
};

