//
// Created by samuel on 18/12/22.
//


#include <btBulletDynamicsCommon.h>
#include <utility>
#include <glm/gtc/type_ptr.hpp>

#include "model/item.h"

Item::Item(std::string name, const std::shared_ptr<Shape> &shape, glm::vec3 position, glm::vec3 scale, float mass)
        : name(std::move(name)), shape(shape), scale(scale) {

    auto *convex_hull_shape = new btConvexHullShape();

    for (auto [x, y, z]: shape->get_vertices())
        convex_hull_shape->addPoint(btVector3(x, y, z));

    collision_shape = convex_hull_shape;

    collision_shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));

    btVector3 local_inertia(0, 0, 0);
    if (mass != 0.f)
        collision_shape->calculateLocalInertia(mass, local_inertia);

    btTransform original_tr;
    original_tr.setIdentity();
    original_tr.setOrigin(btVector3(position.x, position.y, position.z));

    auto *motion_state = new btDefaultMotionState(original_tr);

    btRigidBody::btRigidBodyConstructionInfo body_info(mass, motion_state, collision_shape, local_inertia);

    body = new btRigidBody(body_info);
}

std::shared_ptr<Shape> Item::get_shape() {
    return shape;
}

std::string Item::get_name() {
    return name;
}

btRigidBody *Item::get_body() {
    return body;
}

glm::mat4 Item::model_matrix() {
    btScalar tmp[16];
    btTransform tr;

    body->getMotionState()->getWorldTransform(tr);

    tr.getOpenGLMatrix(tmp);

    return glm::make_mat4(tmp) * glm::scale(glm::mat4(1.f), scale);
}

