#include <stdlib.h>
#include "shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

ShaderProgram::ShaderProgram(const char* vertex_path, const char* frag_path, const char* geo_path) {

        std::string vertex_code;
        std::string frag_code;
        std::string geo_code;

        std::ifstream vertex_file;
        std::ifstream frag_file;
        std::ifstream geo_file;

        vertex_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        frag_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        geo_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);

        try  {
            vertex_file.open(vertex_path);
            frag_file.open(frag_path);

            std::stringstream vertex_stream, frag_stream;
            vertex_stream << vertex_file.rdbuf();
            frag_stream << frag_file.rdbuf();		

            vertex_file.close();
            frag_file.close();

            vertex_code = vertex_stream.str();
            frag_code = frag_stream.str();			

            if(geo_path != nullptr)
            {
                geo_file.open(geo_path);
                std::stringstream gShaderStream;
                gShaderStream << geo_file.rdbuf();
                geo_file.close();
                geo_code = gShaderStream.str();
            }
        }
        catch (std::ifstream::failure& e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
            exit(EXIT_FAILURE);
        }

        const char* vertex_code_cstr = vertex_code.c_str(); 
        const char* frag_code_cstr = frag_code.c_str(); 
        uint32_t vertex, fragment, geometry;

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertex_code_cstr, nullptr);
        glCompileShader(vertex);
        ShaderProgram::check_compile_errors(vertex);

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &frag_code_cstr, nullptr);
        glCompileShader(fragment);
        ShaderProgram::check_compile_errors(fragment);

        if(geo_path) {
            const char* geo_code_cstr = geo_code.c_str(); 
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &geo_code_cstr, nullptr);
            glCompileShader(geometry);
            ShaderProgram::check_compile_errors(geometry);
        }

        id = glCreateProgram();
        glAttachShader(id, vertex);
        glAttachShader(id, fragment);
        if(geo_path)
            glAttachShader(id, geometry);
        glLinkProgram(id);
        check_link_errors(id);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if(geo_path != nullptr)
            glDeleteShader(geometry);
}

void ShaderProgram::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value); 
}

void ShaderProgram::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value); 
}

void ShaderProgram::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value); 
}

void ShaderProgram::setVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
}

void ShaderProgram::setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(id, name.c_str()), x, y); 
}

void ShaderProgram::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
}

void ShaderProgram::setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z); 
}

void ShaderProgram::setVec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
}

void ShaderProgram::setVec4(const std::string &name, float x, float y, float z, float w)  {
    glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w); 
}

void ShaderProgram::setMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::setMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::use() {
    glUseProgram(id);
}

void ShaderProgram::check_link_errors(uint32_t shader)
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

void ShaderProgram::check_compile_errors(uint32_t shader)
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