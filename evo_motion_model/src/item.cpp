//
// Created by samuel on 18/12/22.
//

#include <iostream>
#include <utility>

#include <btBulletDynamicsCommon.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <evo_motion_model/item.h>

#include "./converter.h"

RigidBodyItem::RigidBodyItem(
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

RigidBodyItem::RigidBodyItem(
    std::string name, const std::shared_ptr<Shape> &shape, const glm::vec3 position,
    const glm::quat rotation, const glm::vec3 scale, const float mass, DrawableKind drawable_kind)
    : RigidBodyItem(
          std::move(name), shape,
          glm::translate(glm::mat4(1.f), position) * glm::mat4_cast(rotation), scale, mass,
          drawable_kind) {}

RigidBodyItem::RigidBodyItem(
    std::string name, const std::shared_ptr<Shape> &shape, const glm::vec3 position,
    const glm::vec3 scale, const float mass, DrawableKind drawable_kind)
    : RigidBodyItem(
          std::move(name), shape, position, glm::quat_cast(glm::mat4(1.f)), scale, mass,
          drawable_kind) {}

std::shared_ptr<Shape> RigidBodyItem::get_shape() const { return shape; }

std::string RigidBodyItem::get_name() const { return name; }

btRigidBody *RigidBodyItem::get_body() const { return body; }

glm::mat4 RigidBodyItem::model_matrix() const {
    return model_matrix_without_scale() * glm::scale(glm::mat4(1.f), scale);
}

glm::mat4 RigidBodyItem::model_matrix_without_scale() const {
    btTransform tr;
    body->getMotionState()->getWorldTransform(tr);
    return bullet_to_glm(tr);
}

void RigidBodyItem::reset(const glm::mat4 &main_model_matrix) {
    const btTransform original_tr = glm_to_bullet(main_model_matrix * first_model_matrix);

    body->setWorldTransform(original_tr);
    body->getMotionState()->setWorldTransform(original_tr);

    body->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
    body->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
    body->clearForces();
}

DrawableKind RigidBodyItem::get_drawable_kind() const { return kind; }

void RigidBodyItem::rename(const std::string &new_name) { name = new_name; }

/*
 * Empty item
 */

EmptyItem::EmptyItem(
    const std::string &name, const std::shared_ptr<Shape> &shape, glm::vec3 position,
    glm::quat rotation, glm::vec3 scale, DrawableKind drawable_kind)
    : EmptyItem(
          name, shape, [position]() { return position; }, [rotation]() { return rotation; },
          [scale]() { return scale; }, drawable_kind) {}

EmptyItem::EmptyItem(
    const std::string &name, const std::shared_ptr<Shape> &shape,
    std::function<glm::vec3()> get_position, std::function<glm::quat()> get_rotation,
    std::function<glm::vec3()> get_scale, DrawableKind drawable_kind)
    : name(name), shape(shape), drawable_kind(drawable_kind), get_position(get_position),
      get_rotation(get_rotation), get_scale(get_scale), first_model_matrix(model_matrix()) {}

std::shared_ptr<Shape> EmptyItem::get_shape() const { return shape; }

std::string EmptyItem::get_name() const { return name; }

glm::mat4 EmptyItem::model_matrix() const {
    return model_matrix_without_scale() * glm::scale(glm::mat4(1), get_scale());
}

glm::mat4 EmptyItem::model_matrix_without_scale() const {
    return glm::translate(glm::mat4(1), get_position()) * glm::toMat4(get_rotation());
}

DrawableKind EmptyItem::get_drawable_kind() const { return drawable_kind; }

void EmptyItem::reset(const glm::mat4 &main_model_matrix) {
    const auto [pos, rot, scale] = decompose_model_matrix(main_model_matrix * first_model_matrix);
    get_position = [pos]() { return pos; };
    get_rotation = [rot]() { return rot; };
    get_scale = [scale]() { return scale; };
}
