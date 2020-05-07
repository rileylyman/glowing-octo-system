#pragma once

#include "engine/model.h"
#include "engine/light.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/camera.h"
#include "engine/skybox.h"
#include "engine/vertex.h"
#include "engine/imgui-instance.h"
#include <map>
#include <vector>
#include <string>

struct Scene {

    Scene(std::string filename, VertexBuffer *vertex_buffer);
    ~Scene() {
        delete skybox;
    }

    void add_model(Model *model, ShaderProgram *);
    void add_models(std::vector<Model *> models, ShaderProgram *);

    void add_pointlight(PointLight *light);
    void add_dirlight(DirLight *light);
    void add_spotlight(Spotlight *light);
    void add_lights(std::vector<DirLight *> dirlights, std::vector<PointLight *> pointlights, std::vector<Spotlight *> spotlights);

    void draw(Camera *camera);

    inline ShaderProgram *get_shader(std::string name) { return &shaders[name]; }

    std::vector<Model> get_models() {
        std::vector<Model> ret_models;
        for (std::map<ShaderProgram, std::vector<Model>>::iterator iter = models.begin(); iter != models.end(); iter++) {
            for (Model model : iter->second) {
                ret_models.push_back(model);
            }
        }
        return ret_models;
    }

    std::vector<Mask> get_mesh_masks() {
        uint32_t i = 0;
        std::vector<Mask> mesh_masks;
        for (std::map<ShaderProgram, std::vector<Model>>::iterator iter = models.begin(); iter != models.end(); iter++) {
            for (Model &model: iter->second) {
                for (Mesh &mesh: model.get_meshes()) {
                    mesh.parent_model = &model;
                    mesh_masks.push_back(mesh.get_mask(i));
                    i = (i + 1) % 16;
                }
            }
        }
        return mesh_masks;
    }


    Skybox *skybox;

private:
    std::map<std::string, ShaderProgram> shaders;
    std::map<ShaderProgram, std::vector<Model>> models;

    std::vector<DirLight   > dirlights;
    std::vector<PointLight > pointlights;
    std::vector<Spotlight  > spotlights;
};