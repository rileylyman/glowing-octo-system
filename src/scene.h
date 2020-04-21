#pragma once

#include "model.h"
#include "light.h"
#include "shader.h"
#include "camera.h"
#include "skybox.h"
#include "imgui-instance.h"
#include <map>
#include <vector>

struct Scene {

    Scene() {}

    void set_skybox(Skybox *new_skybox) { skybox = new_skybox; }

    void add_model(Model *model, ShaderProgram *);
    void add_models(std::vector<Model *> models, ShaderProgram *);

    void add_pointlight(PointLight *light);
    void add_dirlight(DirLight *light);
    void add_spotlight(Spotlight *light);
    void add_lights(std::vector<DirLight *> dirlights, std::vector<PointLight *> pointlights, std::vector<Spotlight *> spotlights);

    void draw(Camera *camera);

private:

    Skybox *skybox = nullptr;

    std::map<ShaderProgram*, std::vector<Model *>> models;

    std::vector<DirLight   *> dirlights;
    std::vector<PointLight *> pointlights;
    std::vector<Spotlight  *> spotlights;
};