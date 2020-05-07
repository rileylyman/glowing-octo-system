#include "engine/imgui-instance.h"
#include "engine/physics.h"

bool ImGuiInstance::gui_enabled = false; 
bool ImGuiInstance::render_normals = true; 
bool ImGuiInstance::render_skybox = true;
bool ImGuiInstance::cull_back_face = true;
bool ImGuiInstance::mask_overlay = false;
<<<<<<< HEAD
bool ImGuiInstance::fluid_overlay = false;
bool ImGuiInstance::fsdebug_scalar = false;
bool ImGuiInstance::physics_enabled = false;
=======
bool ImGuiInstance::fluid_overlay = true;
bool ImGuiInstance::fsdebug_scalar = true;
>>>>>>> 6ca36b859bb4ccf17b1831fd99bcf99af6496981
bool ImGuiInstance::draw_model_bb = false;
bool ImGuiInstance::msaa = false;
bool ImGuiInstance::reinhard_hdr = true;
bool ImGuiInstance::wireframe = false;
bool ImGuiInstance::draw_mesh_bb = false;
float ImGuiInstance::camera_speed = 5.0f, ImGuiInstance::camera_sensitivity = 0.004f, ImGuiInstance::camera_fov = 50.0f;
float ImGuiInstance::clear_r = 0.1f;
float ImGuiInstance::clear_g = 0.2f;
float ImGuiInstance::clear_b = 0.3f;
glm::vec3 * ImGuiInstance::camera_pos = nullptr;

ImGuiInstance *ImGuiInstance::instance = nullptr;

ImGuiInstance::ImGuiInstance(GLFWwindow *window, glm::vec3 *cpos) {

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    ImGuiInstance::camera_pos = cpos;

    ImGuiInstance::instance = this;
}

ImGuiInstance::~ImGuiInstance() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiInstance::draw() {

        if (!gui_enabled) {
            return;
        }

        //
        // draw gui
        //
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        ImGui::Begin("Debug Options"); 

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Camera position: (%.1f, %.1f, %.1f)", ImGuiInstance::camera_pos->x, ImGuiInstance::camera_pos->y, ImGuiInstance::camera_pos->z);

        ImGui::Text("Render Settings");

        ImGui::Checkbox("Physics Enabled", &physics_enabled);
        if (ImGui::Button("Tick Physics") && !physics_enabled) {
            Physics::instance->tick();
        }
        ImGui::Checkbox("Normal mapping", &render_normals);
        ImGui::Checkbox("Render skybox", &render_skybox);
        ImGui::Checkbox("Cull Back Face", &cull_back_face);
        ImGui::Checkbox("Show Model Bounding Boxes", &draw_model_bb);
        ImGui::Checkbox("Show Mesh Bounding Boxes", &draw_mesh_bb);
        ImGui::Checkbox("Anti-Aliasing", &msaa);
        ImGui::Checkbox("Reinhard HDR Mapping", &reinhard_hdr);
        ImGui::Checkbox("Wireframe", &wireframe);
        ImGui::Checkbox("Fluid Debug Plane", &fluid_overlay);
        ImGui::Checkbox("Mask Debug Plane", &mask_overlay);
        ImGui::Checkbox("Fluid Debug Scalar Grid?", &fsdebug_scalar);

        if (cull_back_face)  {
            glEnable(GL_CULL_FACE);
        } else  {
            glDisable(GL_CULL_FACE);
        }

        ImGui::Text("Camera Settings");
    
        ImGui::SliderFloat("Camera Speed", &camera_speed, 0.0f, 20.0f);   
        ImGui::SliderFloat("Camera Sensitivity", &camera_sensitivity, 0.0f, 1.0f);   
        ImGui::SliderFloat("Camera Fov", &camera_fov, 0.0f, 180.0f);   

        ImGui::Text("Clear Color");

        ImGui::SliderFloat("r", &clear_r, 0.0f, 1.0f);   
        ImGui::SliderFloat("g", &clear_g, 0.0f, 1.0f);   
        ImGui::SliderFloat("b", &clear_b, 0.0f, 1.0f);   
        
        ImGui::Text("Miscellaneous");

        static int counter = 0;
        if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool ImGuiInstance::mouse_over_imgui() {
    return ImGui::GetIO().WantCaptureMouse;
}