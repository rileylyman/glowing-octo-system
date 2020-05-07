#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <btBulletDynamicsCommon.h>

namespace Fluidsim {
    class Engine;
}

enum RigidBodyType {
    STATIC, 
    KINEMATIC,
    DYNAMIC
};

struct PhysicsObject {
    btRigidBody *body = nullptr;
    glm::vec3 half_extents, bbox_min;

    //TODO: deal with half extents for all meshes
    PhysicsObject(glm::vec3 position, glm::vec3 rotation, RigidBodyType rbtype = RigidBodyType::DYNAMIC, bool gravity = true, glm::vec3 half_extents = glm::vec3(1.0), glm::vec3 bbox_min = glm::vec3(0.0)); 
    ~PhysicsObject();

    //TODO : apply this force over time, or over duration of a tick??
    void apply_force_to_point(glm::vec3 force_in_newtons, glm::vec3 point);
    void apply_force_to_center(glm::vec3 force_in_newtons); 
    void apply_torque(glm::vec3 torque); 

    glm::vec3 position();
    glm::quat orientation();
    glm::mat4 get_model_matrix();

    glm::vec3 get_velocity() {
        btVector3 lin_vel = body->getLinearVelocity();
        return {lin_vel.getX(), lin_vel.getY(), lin_vel.getZ()};
    }

};

struct Physics {

    static Physics* instance;

    double previous_time = 0.0;

    btDefaultCollisionConfiguration* collisionConfig;
    btCollisionDispatcher *dispatcher;
    btBroadphaseInterface *overlappingPairCache;
    btSequentialImpulseConstraintSolver *solver;
    btDiscreteDynamicsWorld *dynamicsWorld; 
    Fluidsim::Engine *fs = nullptr;

    uint32_t pbos[3];

    Physics(); 

    void tick(double frame_time, bool tick = false) {
        dynamicsWorld->stepSimulation(frame_time, tick ? 1 : 10);
    }
};

