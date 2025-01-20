//
// Created by samuel on 18/12/22.
//

#include <utility>

#include <btBulletDynamicsCommon.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/item.h>

#include "./converter.h"

Item::Item(
    std::string name, const std::shared_ptr<Shape> &shape, glm::mat4 model_matrix,
    const glm::vec3 scale, const float mass, const DrawableKind drawable_kind)
    : name(std::move(name)), shape(shape), scale(scale), first_model_matrix(model_matrix),
      kind(drawable_kind) {
    auto *convex_hull_shape = new btConvexHullShape();

    for (auto [x, y, z]: shape->get_vertices()) convex_hull_shape->addPoint(btVector3(x, y, z));

    collision_shape = convex_hull_shape;

    collision_shape->setLocalScaling(glm_to_bullet(scale));

    btVector3 local_inertia(0, 0, 0);
    if (mass != 0.f) collision_shape->calculateLocalInertia(mass, local_inertia);

    btTransform original_tr;
    original_tr.setFromOpenGLMatrix(glm::value_ptr(model_matrix));

    auto *motion_state = new btDefaultMotionState(original_tr);

    const btRigidBody::btRigidBodyConstructionInfo body_info(
        mass, motion_state, collision_shape, local_inertia);

    body = new btRigidBody(body_info);
}

Item::Item(
    std::string name, const std::shared_ptr<Shape> &shape, const glm::vec3 position,
    const glm::quat rotation, const glm::vec3 scale, const float mass, DrawableKind drawable_kind)
    : Item(
          std::move(name), shape,
          glm::translate(glm::mat4(1.f), position) * glm::mat4_cast(rotation), scale, mass,
          drawable_kind) {}

Item::Item(
    std::string name, const std::shared_ptr<Shape> &shape, const glm::vec3 position,
    const glm::vec3 scale, const float mass, DrawableKind drawable_kind)
    : Item(
          std::move(name), shape, position, glm::quat_cast(glm::mat4(1.f)), scale, mass,
          drawable_kind) {}

std::shared_ptr<Shape> Item::get_shape() const { return shape; }

std::string Item::get_name() const { return name; }

btRigidBody *Item::get_body() const { return body; }

glm::mat4 Item::model_matrix() const {
    return model_matrix_without_scale() * glm::scale(glm::mat4(1.f), scale);
}

glm::mat4 Item::model_matrix_without_scale() const {
    btTransform tr;
    body->getMotionState()->getWorldTransform(tr);
    return bullet_to_glm(tr);
}

void Item::reset(const glm::mat4 &main_model_matrix) const {
    const btTransform original_tr = glm_to_bullet(main_model_matrix * first_model_matrix);

    body->setWorldTransform(original_tr);
    body->getMotionState()->setWorldTransform(original_tr);

    body->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
    body->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
    body->clearForces();
}

DrawableKind Item::get_drawable_kind() const { return kind; }
