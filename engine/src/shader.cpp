#include <stdlib.h>
#include "engine/shader.h"
#include "engine/debug.h"
#include "log.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

const uint32_t NR_DL_ATTRS = 4;
const char *DIR_LIGHT_NAMES[] = {
    "u_DirLights[0].direction",
    "u_DirLights[0].ambient",
    "u_DirLights[0].diffuse",
    "u_DirLights[0].specular",
    "u_DirLights[1].direction",
    "u_DirLights[1].ambient",
    "u_DirLights[1].diffuse",
    "u_DirLights[1].specular",
    "u_DirLights[2].direction",
    "u_DirLights[2].ambient",
    "u_DirLights[2].diffuse",
    "u_DirLights[2].specular",
    "u_DirLights[3].direction",
    "u_DirLights[3].ambient",
    "u_DirLights[3].diffuse",
    "u_DirLights[3].specular",
    "u_DirLights[4].direction",
    "u_DirLights[4].ambient",
    "u_DirLights[4].diffuse",
    "u_DirLights[4].specular",
    "u_DirLights[5].direction",
    "u_DirLights[5].ambient",
    "u_DirLights[5].diffuse",
    "u_DirLights[5].specular",
    "u_DirLights[6].direction",
    "u_DirLights[6].ambient",
    "u_DirLights[6].diffuse",
    "u_DirLights[6].specular",
    "u_DirLights[7].direction",
    "u_DirLights[7].ambient",
    "u_DirLights[7].diffuse",
    "u_DirLights[7].specular",
};

const uint32_t NR_PL_ATTRS = 7;
const char *POINT_LIGHT_NAMES[] = {
    "u_PointLights[0].position",
    "u_PointLights[0].ambient",
    "u_PointLights[0].diffuse",
    "u_PointLights[0].specular",
    "u_PointLights[0].constant",
    "u_PointLights[0].linear",
    "u_PointLights[0].quadratic",
    "u_PointLights[1].position",
    "u_PointLights[1].ambient",
    "u_PointLights[1].diffuse",
    "u_PointLights[1].specular",
    "u_PointLights[1].constant",
    "u_PointLights[1].linear",
    "u_PointLights[1].quadratic",
    "u_PointLights[2].position",
    "u_PointLights[2].ambient",
    "u_PointLights[2].diffuse",
    "u_PointLights[2].specular",
    "u_PointLights[2].constant",
    "u_PointLights[2].linear",
    "u_PointLights[2].quadratic",
    "u_PointLights[3].position",
    "u_PointLights[3].ambient",
    "u_PointLights[3].diffuse",
    "u_PointLights[3].specular",
    "u_PointLights[3].constant",
    "u_PointLights[3].linear",
    "u_PointLights[3].quadratic",
};

const uint32_t NR_SL_ATTRS = 10;
const char *SPOTLIGHT_NAMES[] = {
    "u_Spotlights[0].position",
    "u_Spotlights[0].direction",
    "u_Spotlights[0].ambient",
    "u_Spotlights[0].diffuse",
    "u_Spotlights[0].specular",
    "u_Spotlights[0].constant",
    "u_Spotlights[0].linear",
    "u_Spotlights[0].quadratic",
    "u_Spotlights[0].cosPhi",
    "u_Spotlights[0].cosGamma",
    "u_Spotlights[1].position",
    "u_Spotlights[1].direction",
    "u_Spotlights[1].ambient",
    "u_Spotlights[1].diffuse",
    "u_Spotlights[1].specular",
    "u_Spotlights[1].constant",
    "u_Spotlights[1].linear",
    "u_Spotlights[1].quadratic",
    "u_Spotlights[1].cosPhi",
    "u_Spotlights[1].cosGamma",
    "u_Spotlights[2].position",
    "u_Spotlights[2].direction",
    "u_Spotlights[2].ambient",
    "u_Spotlights[2].diffuse",
    "u_Spotlights[2].specular",
    "u_Spotlights[2].constant",
    "u_Spotlights[2].linear",
    "u_Spotlights[2].quadratic",
    "u_Spotlights[2].cosPhi",
    "u_Spotlights[2].cosGamma",
    "u_Spotlights[3].position",
    "u_Spotlights[3].direction",
    "u_Spotlights[3].ambient",
    "u_Spotlights[3].diffuse",
    "u_Spotlights[3].specular",
    "u_Spotlights[3].constant",
    "u_Spotlights[3].linear",
    "u_Spotlights[3].quadratic",
    "u_Spotlights[3].cosPhi",
    "u_Spotlights[3].cosGamma",
};


ShaderProgram::ShaderProgram(
    std::string vertex_path, 
    std::string frag_path, 
    std::string *geo_path)
{

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

        if (geo_path != nullptr) {
            geo_file.open(*geo_path);
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
    glCheckError();
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value); 
    glCheckError();
}

void ShaderProgram::setInt(const std::string &name, int value) const {
    if (value == -1) {
        return;
    }
    glCheckError();
    GLuint location = glGetUniformLocation(id, name.c_str());
    glCheckError();
    glUniform1i(location, (GLint)value); 
    glCheckError();
}

void ShaderProgram::setFloat(const std::string &name, float value) const {
    glCheckError();
    glUniform1f(glGetUniformLocation(id, name.c_str()), value); 
    glCheckError();
}

void ShaderProgram::setVec2(const std::string &name, const glm::vec2 &value) const {
    glCheckError();
    glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
    glCheckError();
}

void ShaderProgram::setVec2(const std::string &name, float x, float y) const {
    glCheckError();
    glUniform2f(glGetUniformLocation(id, name.c_str()), x, y); 
    glCheckError();
}

void ShaderProgram::setVec3(const std::string &name, const glm::vec3 &value) const {
    glCheckError();
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
    glCheckError();
}

void ShaderProgram::setVec3(const std::string &name, float x, float y, float z) const {
    glCheckError();
    glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z); 
    glCheckError();
}

void ShaderProgram::setVec4(const std::string &name, const glm::vec4 &value) const {
    glCheckError();
    glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
    glCheckError();
}

void ShaderProgram::setVec4(const std::string &name, float x, float y, float z, float w)  {
    glCheckError();
    glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w); 
    glCheckError();
}

void ShaderProgram::setMat2(const std::string &name, const glm::mat2 &mat) const {
    glCheckError();
    glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    glCheckError();
}

void ShaderProgram::setMat3(const std::string &name, const glm::mat3 &mat) const {
    glCheckError();
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    glCheckError();
}

void ShaderProgram::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glCheckError();
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    glCheckError();
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

void ShaderProgram::bind_lights(
    std::vector<DirLight> dir_lights,
    std::vector<PointLight> point_lights,
    std::vector<Spotlight> spot_lights)
{
    use();
    if (dir_lights.size() > max_nr_dir_lights || point_lights.size() > max_nr_point_lights || spot_lights.size() > max_nr_spotlights) {
        std::cout << "Too many lights!\n";
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < dir_lights.size(); i++) {
        setVec3(DIR_LIGHT_NAMES[i*NR_DL_ATTRS + 0], dir_lights[i].direction);
        setVec3(DIR_LIGHT_NAMES[i*NR_DL_ATTRS + 1], dir_lights[i].ambient);
        setVec3(DIR_LIGHT_NAMES[i*NR_DL_ATTRS + 2], dir_lights[i].diffuse);
        setVec3(DIR_LIGHT_NAMES[i*NR_DL_ATTRS + 3], dir_lights[i].specular);
    }
    setInt("u_NrDirLights", dir_lights.size());
    for (int i = 0; i < point_lights.size(); i++) {
        setVec3( POINT_LIGHT_NAMES[i*NR_PL_ATTRS + 0], point_lights[i].position);
        setVec3( POINT_LIGHT_NAMES[i*NR_PL_ATTRS + 1], point_lights[i].ambient);
        setVec3( POINT_LIGHT_NAMES[i*NR_PL_ATTRS + 2], point_lights[i].diffuse);
        setVec3( POINT_LIGHT_NAMES[i*NR_PL_ATTRS + 3], point_lights[i].specular);
        setFloat(POINT_LIGHT_NAMES[i*NR_PL_ATTRS + 4], point_lights[i].constant);
        setFloat(POINT_LIGHT_NAMES[i*NR_PL_ATTRS + 5], point_lights[i].linear);
        setFloat(POINT_LIGHT_NAMES[i*NR_PL_ATTRS + 6], point_lights[i].quadratic);
    }
    setInt("u_NrPointLights", point_lights.size());
    for (int i = 0; i < spot_lights.size(); i++) {
        setVec3( SPOTLIGHT_NAMES[i*NR_SL_ATTRS + 0], spot_lights[i].position);
        setVec3( SPOTLIGHT_NAMES[i*NR_SL_ATTRS + 1], spot_lights[i].direction);
        setVec3( SPOTLIGHT_NAMES[i*NR_SL_ATTRS + 2], spot_lights[i].ambient);
        setVec3( SPOTLIGHT_NAMES[i*NR_SL_ATTRS + 3], spot_lights[i].diffuse);
        setVec3( SPOTLIGHT_NAMES[i*NR_SL_ATTRS + 4], spot_lights[i].specular);
        setFloat(SPOTLIGHT_NAMES[i*NR_SL_ATTRS + 5], spot_lights[i].constant);
        setFloat(SPOTLIGHT_NAMES[i*NR_SL_ATTRS + 6], spot_lights[i].linear);
        setFloat(SPOTLIGHT_NAMES[i*NR_SL_ATTRS + 7], spot_lights[i].quadratic);
        setFloat(SPOTLIGHT_NAMES[i*NR_SL_ATTRS + 8], spot_lights[i].cosPhi);
        setFloat(SPOTLIGHT_NAMES[i*NR_SL_ATTRS + 9], spot_lights[i].cosGamma);
    }
    setInt("u_NrSpotlights", spot_lights.size());
}