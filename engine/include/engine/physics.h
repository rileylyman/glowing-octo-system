#pragma once

#include <GLFW/glfw3.h>
#include <reactphysics3d.h>
#include <glm/glm.hpp>
#include <vector>

namespace rp3d = reactphysics3d;

enum RigidBodyType {
    STATIC, 
    KINEMATIC,
    DYNAMIC
};

struct PhysicsObject {
    rp3d::RigidBody *body;
    rp3d::Transform previous_transform, current_transform;

    PhysicsObject(glm::vec3 position, RigidBodyType rbtype = RigidBodyType::DYNAMIC, bool gravity = true); 
    ~PhysicsObject();

    void set_bounciness(double bounciness);
    void set_friction_coefficient(double coeff);

    //TODO : apply this force over time, or over duration of a tick??
    void apply_force_to_point(glm::vec3 force_in_newtons, glm::vec3 world_space_point);
    void apply_force_to_center(glm::vec3 force_in_newtons);

    glm::vec3 position();
    glm::quat orientation();

};

struct Physics {
    static rp3d::DynamicsWorld world;
    static uint32_t num_velocity_solver_iters, num_position_solver_iters;

    static double previous_time;
    static double accumulator;

    static std::vector<PhysicsObject *> physics_objects;

    static void init() {
        world.setNbIterationsVelocitySolver(num_velocity_solver_iters);
        world.setNbIterationsPositionSolver(num_position_solver_iters);

        previous_time = glfwGetTime();
    }

    static void tick() {
        const double timestep = 1.0 / 60.0;

        double current_time = glfwGetTime(); 
        double frame_time = current_time - previous_time;
        previous_time = current_time;

        accumulator += frame_time;
        while (accumulator >= timestep) {
            for (PhysicsObject *po : physics_objects) {
                po->previous_transform = po->current_transform;
            }
            world.update(timestep);
            accumulator -= timestep;
        }

        double alpha = abs(accumulator / timestep);

        for (PhysicsObject *po : physics_objects) {
            po->current_transform = rp3d::Transform::interpolateTransforms(po->previous_transform, po->body->getTransform(), alpha);
        }
    }
};

