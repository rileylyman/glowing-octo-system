#include <nlohmann/json.hpp>
#include "engine/scene.h"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

Scene::Scene(std::string filename, VertexBuffer *vertex_buffer) {
    json scene_json;
    std::ifstream i(filename);
    i >> scene_json;

    if (scene_json.find("skybox") == scene_json.end()) {
        std::cout << "SCENE PARSE ERROR: No skybox specified!" << std::endl;
        throw false;
    }
    if (
        scene_json["skybox"].find("right") == scene_json["skybox"].end() ||
        scene_json["skybox"].find("left") == scene_json["skybox"].end() ||
        scene_json["skybox"].find("top") == scene_json["skybox"].end() ||
        scene_json["skybox"].find("bottom") == scene_json["skybox"].end() ||
        scene_json["skybox"].find("front") == scene_json["skybox"].end() ||
        scene_json["skybox"].find("back") == scene_json["skybox"].end() 
    ) {
        std::cout << "SCENE PARSE ERROR: Skybox needs right,left,top,bottom,front,back texture paths" << std::endl;
        throw false;
    }
    skybox = new Skybox(
        {
            (std::string)scene_json["skybox"]["right"],
            (std::string)scene_json["skybox"]["left"],
            (std::string)scene_json["skybox"]["top"],
            (std::string)scene_json["skybox"]["bottom"],
            (std::string)scene_json["skybox"]["front"],
            (std::string)scene_json["skybox"]["back"]
        },
        "src/shaders/skybox.vert",
        "src/shaders/skybox.frag"
    );

    if (scene_json.find("shaders") == scene_json.end()) {
        std::cout << "SCENE PARSE ERROR: Field 'shaders' not found" << std::endl;
        throw false;
    }
    for (json shader_json : scene_json["shaders"]) {
        if (shader_json.find("name") == shader_json.end()) {
            std::cout << "SCENE PARSE ERROR: Field 'shaders.name' not found" << std::endl;
            throw false;
        }
        std::string shader_name = shader_json["name"];
        if (shaders.count(shader_name) > 0) {
            std::cout << "ERROR: Scene defines shader " << shader_name << " twice!\n";
            throw false;
        }
        if (shader_json.find("vertexPath") == shader_json.end()) {
            std::cout << "SCENE PARSE ERROR: Field 'shaders.vertexPath' not found" << std::endl;
            throw false;
        }
        if (shader_json.find("fragmentPath") == shader_json.end()) {
            std::cout << "SCENE PARSE ERROR: Field 'shaders.fragmentPath' not found" << std::endl;
            throw false;
        }
        std::string vert_path = shader_json["vertexPath"];
        std::string frag_path = shader_json["fragmentPath"];
        shaders[shader_name] = ShaderProgram(vert_path, frag_path);
    }

    if (scene_json.find("models") == scene_json.end()) {
        std::cout << "SCENE PARSE ERROR: Field 'models' not found" << std::endl;
        throw false;
    }
    for (json model_json : scene_json["models"]) {
        if (model_json.find("name") == model_json.end()) {
            std::cout << "SCENE PARSE ERROR: Field 'models.name' not found" << std::endl;
            throw false;
        }
        std::string model_name = model_json["name"];
        if (model_json.find("shader") == model_json.end()) {
            std::cout << "SCENE PARSE ERROR: Field 'models.shader' not found" << std::endl;
            throw false;
        }
        std::string shader_ref = model_json["shader"];
        ShaderProgram referenced_shader = shaders[shader_ref];
        if (models.count(referenced_shader) == 0) {
            models[referenced_shader] = std::vector<Model>();
        }
        if (model_json.find("shader") == model_json.end()) {
            std::cout << "SCENE PARSE ERROR: Field 'models.shader' not found" << std::endl;
            throw false;
        }
        std::string path = model_json["modelPath"];
        MeshShaderType shader_type;
        std::string shader_type_string = model_json["shaderType"];
        if (shader_type_string == "blinnphong_textured") {
            shader_type = BP_TEXTURED;
        }
        else if (shader_type_string == "pbr_textured") {
            shader_type = PBR_TEXTURED;
        } 
        else {
            std::cout << "Unrecognized shaderType for model " << model_name << " in scene " << filename << "\n";
            throw false;
        }

        bool height_normals = model_json["heightNormals"];

        //Model model(vertex_buffer, path, shader_type, 0, height_normals);
        //models[shaders[shader_ref]].push_back(model);

        models[shaders[shader_ref]].emplace_back(vertex_buffer, path, shader_type, 0, height_normals);
    }

    if (scene_json.find("lights") == scene_json.end()) {
        std::cout << "SCENE PARSE ERROR: Field 'lights' not found" << std::endl;
        throw false;
    }
    for (json light_json : scene_json["lights"]) {
        if (light_json.find("type") == light_json.end()) {
            std::cout << "SCENE PARSE ERROR: Field 'lights.type' not found" << std::endl;
            throw false;
        }
        if (light_json["type"] == "directional") {
            DirLight light;
            if (light_json.contains("diffuse")) {
                std::vector<float> diff = light_json["diffuse"];
                light.diffuse = glm::vec3(diff[0], diff[1], diff[2]);
            }
            if (!light_json.contains("direction")) {
                std::cout << "ERROR: light " << light_json["name"] << " specified without a direction!\n";
                throw false;
            }
            std::vector<float> dir = light_json["direction"];
            light.direction = glm::vec3(dir[0], dir[1], dir[2]);

            dirlights.push_back(light);
        } else if (light_json["type"] == "point") {
            PointLight light;
            if (!light_json.contains("position")) {
                std::cout << "ERROR: light " << light_json["name"] << " specified without a position!\n";
                throw false;
            }
            std::vector<float> pos = light_json["position"];
            light.position = glm::vec3(pos[0], pos[1], pos[2]);

            pointlights.push_back(light);
        } else {
            std::cout << "Unsupported light type \"" << light_json["type"] << "\" in " << filename << "\n";
            throw false;
        }
    }
}

void Scene::add_model(Model *new_model, ShaderProgram *shader) {
    if (models.count(*shader) == 0) {
        models[*shader] = std::vector<Model>();
    }
    models[*shader].push_back(*new_model);
}

void Scene::add_models(std::vector<Model *> new_models, ShaderProgram *shader) {
    if (models.count(*shader) == 1) {
        models[*shader].reserve(models[*shader].size() + new_models.size());
    }
    for (Model *new_model : new_models) {
        add_model(new_model, shader);
    }
}

void Scene::add_pointlight(PointLight *light) {
    pointlights.push_back(*light);
}

void Scene::add_dirlight(DirLight *light) {
    dirlights.push_back(*light);
}

void Scene::add_spotlight(Spotlight *light) {
    spotlights.push_back(*light);
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
    
    for (std::map<ShaderProgram, std::vector<Model>>::iterator iter = models.begin(); iter != models.end(); iter++) {
        ShaderProgram shader = iter->first;
        std::vector<Model> models_to_render = iter->second;

        shader.use();
        shader.bind_lights(dirlights, pointlights, spotlights);

        if (ImGuiInstance::reinhard_hdr) {
            shader.setBool("u_Reinhard", true);
        } else {
            shader.setBool("u_Reinhard", false);
        }

        for (Model model : models_to_render) {
            model.draw(shader, camera);
            if (ImGuiInstance::draw_model_bb) model.draw_bounding_box(camera);

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