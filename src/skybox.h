#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include "shader.h"
#include "texture.h"
#include "camera.h"

struct Skybox {

    Skybox(std::vector<std::string> texture_names, std::string vshader, std::string fshader);
    ~Skybox();
    void draw(Camera *camera);

private:
    ShaderProgram shader;
    Cubemap cubemap;
    uint32_t vao, vbo;
};