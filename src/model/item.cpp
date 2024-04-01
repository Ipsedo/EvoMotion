//
// Created by samuel on 18/12/22.
//


#include <btBulletDynamicsCommon.h>
#include <utility>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

#include "item.h"

Item::Item(std::string name, const std::shared_ptr<Shape> &shape, glm::mat4 model_matrix, glm::vec3 scale, float mass)
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
    original_tr.setFromOpenGLMatrix(glm::value_ptr(model_matrix));
    curr_model_matrix = model_matrix;

    auto *motion_state = new btDefaultMotionState(original_tr);

    btRigidBody::btRigidBodyConstructionInfo body_info(mass, motion_state, collision_shape, local_inertia);

    body = new btRigidBody(body_info);

}

Item::Item(std::string name, const std::shared_ptr<Shape> &shape, glm::vec3 position, glm::quat rotation,
           glm::vec3 scale, float mass)
    : Item(std::move(name), shape, glm::translate(glm::mat4(1.f), position) * glm::mat4_cast(rotation), scale, mass) {


}

Item::Item(std::string name, const std::shared_ptr<Shape> &shape, glm::vec3 position, glm::vec3 scale, float mass)
    : Item(std::move(name), shape, position, glm::quat_cast(glm::mat4(1.f)), scale, mass) {

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

    curr_model_matrix = glm::make_mat4(tmp);

    return curr_model_matrix * glm::scale(glm::mat4(1.f), scale);
}

glm::mat4 Item::get_last_model_matrix() {
    return curr_model_matrix;
}

std::tuple<Item, btHingeConstraint *>
Item::attach_item_hinge(glm::mat4 model_in_parent, glm::mat4 attach_in_parent, glm::mat4 attach_in_sub,
                        glm::vec3 hinge_axis, std::string sub_name,
                        const std::shared_ptr<Shape> &sub_shape, glm::vec3 sub_scale, float mass) {
    glm::mat4 sub_mat = get_last_model_matrix() * model_in_parent;

    Item sub_item(std::move(sub_name), sub_shape, sub_mat, sub_scale, mass);

    btVector3 axis(hinge_axis.x, hinge_axis.y, hinge_axis.z);
    glm::vec3 pos_in_parent = glm::vec3(attach_in_parent * glm::vec4(0, 0, 0, 1));
    glm::vec3 pos_in_sub = glm::vec3(attach_in_sub * glm::vec4(0, 0, 0, 1));

    auto hinge = new btHingeConstraint(
        *get_body(),
        *sub_item.get_body(),
        btVector3(pos_in_parent.x, pos_in_parent.y, pos_in_parent.z),
        btVector3(pos_in_sub.x, pos_in_sub.y, pos_in_sub.z),
        axis, axis
    );

    get_body()->setIgnoreCollisionCheck(sub_item.get_body(), true);
    hinge->setLimit(0, M_PI);

    return {
        sub_item,
        hinge
    };
}

std::tuple<Item, btFixedConstraint *>
Item::attach_item_fixed(glm::mat4 model_in_parent, glm::mat4 attach_in_parent, glm::mat4 attach_in_sub,
                        std::string sub_name,
                        const std::shared_ptr<Shape> &sub_shape, glm::vec3 sub_scale, float mass) {
    glm::mat4 sub_mat = get_last_model_matrix() * model_in_parent;

    Item sub_item(std::move(sub_name), sub_shape, sub_mat, sub_scale, mass);

    btTransform parent_tr;
    parent_tr.setFromOpenGLMatrix(glm::value_ptr(attach_in_parent));
    btTransform sub_tr;
    sub_tr.setFromOpenGLMatrix(glm::value_ptr(attach_in_sub));

    get_body()->setIgnoreCollisionCheck(sub_item.get_body(), true);

    auto fixed = new btFixedConstraint(
        *get_body(),
        *sub_item.get_body(),
        parent_tr,
        sub_tr
    );

    return {
        sub_item,
        fixed
    };
}



