#pragma once

#include <glad/glad.h>
#include <stdint.h>
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include "light.h"

enum UniformName {
    // Vertex
    TRANSFORM = 0,
    CAMERA_POS,
    MODEL,
    NORMAL_MATRIX,

    //Fragment
    MATERIAL,
    LIGHT,
    CONTAINER,
    SMILEY
};

struct ShaderProgram {
    const int max_nr_dirlights = 1;
    const int max_nr_pointlights = 16;
    const int max_nr_spotlights = 1;

    uint32_t id;
    std::vector<UniformName> uniforms;

    ShaderProgram(std::vector<UniformName> uniforms, const char* vertex_path, const char* frag_path, const char* geo_path=nullptr); 

    void use();
    void bind_lights(glm::mat4 view, std::vector<DirLight*> dir_lights, std::vector<PointLight*> point_lights, std::vector<Spotlight*> spot_lights); 
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
    static void check_link_errors(uint32_t id);
    static void check_compile_errors(uint32_t id);
};
