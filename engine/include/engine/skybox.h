#pragma once

#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/camera.h"

struct Skybox {

    Skybox(std::vector<std::string> texture_names, std::string vshader, std::string fshader);
    ~Skybox();
    void draw(Camera *camera);

private:
    ShaderProgram shader;
    Cubemap cubemap;
    uint32_t vao, vbo;
};