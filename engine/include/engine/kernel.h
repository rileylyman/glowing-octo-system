#pragma once

#include <glad/glad.h>
#include <stdint.h>
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include "light.h"
#include "texture.h"

//
// Represents the final, linked shader program that is used by the
// graphics pipeline to render images
//
struct KernelProgram {

    //
    // Default constructor so that other things work... not to be used to create an
    // actual shader!
    //
    KernelProgram(): id(UINT32_MAX) {}
    KernelProgram& operator=(const KernelProgram& other) {
        id = other.id;
    }

    //
    // Specify the path for the compute shader
    // to create a shader.
    //
    KernelProgram(std::string kernel_path); 

    //
    // Set the OpenGl state machine's active shader to this one, meaning that
    // all draw calls will now invoke this shader.
    //
    void use();

    friend bool operator<(const KernelProgram first, const KernelProgram second) {
        return first.id < second.id;
    }

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
    // Helper functions to check for compilation and link errors
    // of the supplied shaders
    //
    static void check_link_errors(uint32_t id);
    static void check_compile_errors(uint32_t id);
};

