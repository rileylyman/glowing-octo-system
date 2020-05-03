#include "engine/physics.h"
#include "glm/gtc/quaternion.hpp"

rp3d::DynamicsWorld Physics::world = rp3d::DynamicsWorld({0.0, -9.81, 0.0});
uint32_t Physics::num_position_solver_iters = 5;
uint32_t Physics::num_velocity_solver_iters = 10;
double Physics::accumulator = 0.0;
std::vector<PhysicsObject *> Physics::physics_objects;

PhysicsObject::PhysicsObject(glm::vec3 position, RigidBodyType rbtype, bool gravity) {
    rp3d::Transform transform({position.x, position.y, position.z}, rp3d::Quaternion::identity());
    body = Physics::world.createRigidBody(transform);

    current_transform = transform;
    previous_transform = transform;

    switch (rbtype) {
        case RigidBodyType::DYNAMIC:
            body->setType(rp3d::BodyType::DYNAMIC);
        break;
        case RigidBodyType::KINEMATIC:
            body->setType(rp3d::BodyType::KINEMATIC);
        break;
        case RigidBodyType::STATIC:
            body->setType(rp3d::BodyType::STATIC);
        break;
    }

    body->enableGravity(gravity);

    Physics::physics_objects.push_back(this);
}

PhysicsObject::~PhysicsObject() {
    //TODO: remove self from Physics::physics_objects
    Physics::world.destroyRigidBody(body);
}

void PhysicsObject::set_bounciness(double bounciness) {
    assert(bounciness >= 0.0 && bounciness <= 1.0);
    rp3d::Material &material = body->getMaterial();
    material.setBounciness(bounciness);
}

void PhysicsObject::set_friction_coefficient(double coeff) {
    assert(coeff >= 0.0 && coeff <= 1.0);
    rp3d::Material &material = body->getMaterial();
    material.setFrictionCoefficient(coeff);
}


void PhysicsObject::apply_force_to_point(glm::vec3 force_in_newtons, glm::vec3 world_space_point) {
    body->applyForce({force_in_newtons.x, force_in_newtons.y, force_in_newtons.z}, {world_space_point.x, world_space_point.y, world_space_point.z});
}

void PhysicsObject::apply_force_to_center(glm::vec3 force_in_newtons) {
    body->applyForceToCenterOfMass({force_in_newtons.x, force_in_newtons.y, force_in_newtons.z});
}

glm::vec3 PhysicsObject::position() {
    rp3d::Vector3 pos = current_transform.getPosition();
    return {pos.x, pos.y, pos.z};
}

glm::quat PhysicsObject::orientation() {
    rp3d::Quaternion orientation = current_transform.getOrientation();
    glm::quat q;
    q.w = orientation.w;
    q.x = orientation.x;
    q.y = orientation.y;
    q.z = orientation.z;
    return q;
}