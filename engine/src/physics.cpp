#include "engine/physics.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

Physics *Physics::instance = nullptr;

Physics::Physics() {

    collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfig);
    overlappingPairCache = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfig);
    dynamicsWorld->setGravity(btVector3(0, -10, 0));

    previous_time = glfwGetTime();

    instance = this;
}

PhysicsObject::PhysicsObject(glm::vec3 position, glm::vec3 rotation, RigidBodyType rbtype, bool gravity, glm::vec3 half_extents_, glm::vec3 bbox_min_) {
    glm::quat quaternion = glm::quat(rotation); 
    bbox_min = bbox_min_;
    half_extents = half_extents_;

    btCollisionShape *colShape = new btBoxShape(btVector3(btScalar(half_extents_.x), btScalar(half_extents_.y), btScalar(half_extents_.z)));
    btTransform startTransform;

    startTransform.setOrigin(btVector3(position.x, position.y, position.z));
    startTransform.setRotation(btQuaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w));

    btScalar mass(1.f);
    btVector3 localInertia(0, 0, 0);
    if (rbtype == RigidBodyType::DYNAMIC) {
        colShape->calculateLocalInertia(mass, localInertia);
    } else if (rbtype == RigidBodyType::STATIC) {
        mass = btScalar(0.f);
    } else {
        std::cout << "ERROR: kinematic types not yet implemented" << std::endl;
        throw false;
    }

    btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
    body = new btRigidBody(rbInfo);

    Physics::instance->dynamicsWorld->addRigidBody(body);
}

PhysicsObject::~PhysicsObject() {
    //TODO: cleanup
}

void PhysicsObject::apply_force_to_point(glm::vec3 force_in_newtons, glm::vec3 point) {
    body->applyForce({force_in_newtons.x, force_in_newtons.y, force_in_newtons.z}, {point.x, point.y, point.z});
}

void PhysicsObject::apply_force_to_center(glm::vec3 force_in_newtons) {
    body->applyForce({force_in_newtons.x, force_in_newtons.y, force_in_newtons.z}, {0.0, 0.0, 0.0});
}

void PhysicsObject::apply_torque(glm::vec3 torque) {
    body->applyTorque({torque.x, torque.y, torque.z});
}

glm::vec3 PhysicsObject::position() {
    btTransform trans;
    body->getMotionState()->getWorldTransform(trans);

    btVector3 pos = trans.getOrigin();
    return {pos.getX(), pos.getY(), pos.getZ()};
}

glm::quat PhysicsObject::orientation() {
    btTransform trans;
    body->getMotionState()->getWorldTransform(trans);

    btQuaternion orientation = trans.getRotation();
    glm::quat q;
    q.x = orientation.getX();
    q.y = orientation.getY();
    q.z = orientation.getZ();
    q.w = orientation.getW();
    return q;
}

glm::mat4 PhysicsObject::get_model_matrix() {

    glm::vec3 pos = position();
    glm::quat ori = orientation();
    glm::vec3 euler = glm::eulerAngles(ori) * 3.14159f / 180.f;

    //std::cout << "Pos: (" << pos.x << ", " << pos.y << ", " << pos.z << ") Rot: (" << euler.x << ", " << euler.y << ", " << euler.z << ")" << std::endl;

    float matrix[16];
    btTransform trans;
    body->getMotionState()->getWorldTransform(trans);

    trans.setOrigin(btVector3(pos.x, pos.y, pos.z));
    trans.getOpenGLMatrix(matrix);

    return glm::make_mat4(matrix);
}
