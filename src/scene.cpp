#include "scene.h"

void Scene::add_model(Model *new_model, ShaderProgram *shader) {
    if (models.count(shader) == 0) {
        models[shader] = std::vector<Model *>();
    }
    models[shader].push_back(new_model);
}

void Scene::add_models(std::vector<Model *> new_models, ShaderProgram *shader) {
    if (models.count(shader) == 1) {
        models[shader].reserve(models[shader].size() + new_models.size());
    }
    for (Model *new_model : new_models) {
        add_model(new_model, shader);
    }
}

void Scene::add_pointlight(PointLight *light) {
    pointlights.push_back(light);
}

void Scene::add_dirlight(DirLight *light) {
    dirlights.push_back(light);
}

void Scene::add_spotlight(Spotlight *light) {
    spotlights.push_back(light);
}

void Scene::add_lights(std::vector<DirLight *> new_dirlights, std::vector<PointLight *> new_pointlights, std::vector<Spotlight *> new_spotlights) {

    dirlights.reserve(new_dirlights.size() + dirlights.size());
    pointlights.reserve(new_pointlights.size() + pointlights.size());
    spotlights.reserve(new_spotlights.size() + spotlights.size());

    for (DirLight *new_dl : new_dirlights) {
        add_dirlight(new_dl);
    }

    for (PointLight *new_pl : new_pointlights) {
        add_pointlight(new_pl);
    }

    for (Spotlight *new_sl : new_spotlights) {
        add_spotlight(new_sl);
    }
}

void Scene::draw(Camera *camera) {
    
    for (std::map<ShaderProgram *, std::vector<Model *>>::iterator iter = models.begin(); iter != models.end(); iter++) {
        ShaderProgram *shader = iter->first;
        std::vector<Model *> models_to_render = iter->second;

        shader->use();
        shader->bind_lights(dirlights, pointlights, spotlights);

        if (ImGuiInstance::reinhard_hdr) {
            shader->setBool("u_Reinhard", true);
        } else {
            shader->setBool("u_Reinhard", false);
        }

        for (Model *model : models_to_render) {
            model->draw(*shader, camera);
            if (ImGuiInstance::draw_model_bb) model->draw_bounding_box(camera);

        }

        draw_ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 100.0f, 0.0f), camera->projection(), camera->view());

    }

    //
    // After rendering ALL objects, then we can reset the polygon mode back to GL_FILL
    // so that the skybox renders correctly regardless of whether or not we have
    // wireframe enabled.
    //
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (ImGuiInstance::render_skybox)
        skybox->draw(camera);

}