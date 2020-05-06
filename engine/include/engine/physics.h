#pragma once

#include <GLFW/glfw3.h>
#include "reactphysics3d.h"
#include <glm/glm.hpp>
#include <vector>

using namespace reactphysics3d;

enum RigidBodyType {
    STATIC, 
    KINEMATIC,
    DYNAMIC
};

struct PhysicsObject {
    rp3d::RigidBody *body = nullptr;
    rp3d::Transform previous_transform = rp3d::Transform::identity(), current_transform = rp3d::Transform::identity();

    PhysicsObject(glm::vec3 position, glm::vec3 rotation, RigidBodyType rbtype = RigidBodyType::DYNAMIC, bool gravity = true); 
    ~PhysicsObject();

    void set_bounciness(double bounciness);
    void set_friction_coefficient(double coeff);

    //TODO : apply this force over time, or over duration of a tick??
    void apply_force_to_point(glm::vec3 force_in_newtons, glm::vec3 world_space_point);
    void apply_force_to_center(glm::vec3 force_in_newtons);

    glm::vec3 position();
    glm::quat orientation();
    glm::mat4 get_model_matrix();

};

struct Physics {

    static Physics* instance;

    rp3d::DynamicsWorld *world = nullptr;
    uint32_t num_velocity_solver_iters = 10, num_position_solver_iters = 5;

    double previous_time = 0.0;
    double accumulator = 0.0;

    std::vector<PhysicsObject *> physics_objects;

    Physics() {
        world = new rp3d::DynamicsWorld({0.0, -9.81, 0.0});

        world->setNbIterationsVelocitySolver(num_velocity_solver_iters);
        world->setNbIterationsPositionSolver(num_position_solver_iters);

        previous_time = glfwGetTime();

        instance = this;
    }

    void tick() {
        const double timestep = 1.0 / 60.0;

        double current_time = glfwGetTime(); 
        double frame_time = current_time - previous_time;
        previous_time = current_time;

        accumulator += frame_time;
        while (accumulator >= timestep) {
            for (PhysicsObject *po : physics_objects) {
                po->previous_transform = po->current_transform;
            }
            world->update(timestep);
            accumulator -= timestep;
        }

        double alpha = accumulator / (double)timestep;

        for (PhysicsObject *po : physics_objects) {
            po->current_transform = rp3d::Transform::interpolateTransforms(po->previous_transform, po->body->getTransform(), rp3d::decimal(alpha));
        }
    }
};

