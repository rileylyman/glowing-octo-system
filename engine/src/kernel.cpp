#include <stdlib.h>
#include "engine/kernel.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

KernelProgram::KernelProgram(
    std::string kernel_path)
{

    std::string kernel_code;

    std::ifstream kernel_file;

    kernel_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);


    try  {
        kernel_file.open(kernel_path);

        std::stringstream kernel_stream;
        kernel_stream << kernel_file.rdbuf();

        kernel_file.close();

        kernel_code = kernel_stream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        exit(EXIT_FAILURE);
    }

    const char* kernel_code_cstr = kernel_code.c_str();
    uint32_t kernel;

    kernel = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(kernel, 1, &kernel_code_cstr, nullptr);
    glCompileShader(kernel);
    KernelProgram::check_compile_errors(kernel);

    id = glCreateProgram();
    glAttachShader(id, kernel);
    glLinkProgram(id);
    check_link_errors(id);

    glDeleteShader(kernel); //TODO: verify the validity of needing this
}

void KernelProgram::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value); 
}

void KernelProgram::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value); 
}

void KernelProgram::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value); 
}

void KernelProgram::setVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
}

void KernelProgram::setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(id, name.c_str()), x, y); 
}

void KernelProgram::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
}

void KernelProgram::setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z); 
}

void KernelProgram::setVec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
}

void KernelProgram::setVec4(const std::string &name, float x, float y, float z, float w)  {
    glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w); 
}

void KernelProgram::setMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void KernelProgram::setMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void KernelProgram::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void KernelProgram::use() {
    glUseProgram(id);
}

void KernelProgram::check_link_errors(uint32_t shader)
{
    GLint success;
    GLchar infoLog[1024];
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "Error linking shader program:\n" << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }
}

void KernelProgram::check_compile_errors(uint32_t shader)
{
    GLint success;
    GLchar infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "Error compiling shader:\n" << infoLog << std::endl;
    }
}