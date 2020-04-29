#pragma once

#include "engine/model.h"
#include "engine/light.h"
#include "engine/shader.h"
#include "engine/camera.h"
#include "engine/skybox.h"
#include "engine/imgui-instance.h"
#include <map>
#include <vector>
#include <string>

struct Scene {

    Scene() {}
    Scene(std::string filename);

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