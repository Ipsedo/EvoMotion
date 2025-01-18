//
// Created by samuel on 16/01/25.
//

#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/robot_builder.h>

#include "../converter.h"

BuilderMember::BuilderMember(
    const std::string &name, ShapeKind shape_kind, glm::vec3 center_pos, glm::quat rotation,
    glm::vec3 scale, float mass, float friction)
    : shape_to_path(
          {{SPHERE, "./resources/obj/sphere.obj"},
           {CUBE, "./resources/obj/cube.obj"},
           {CYLINDER, "./resources/obj/cylinder.obj"},
           {FEET, "./resources/obj/feet.obj"}}),
      shape(shape_kind), name(name),
      member(
          name, std::make_shared<ObjShape>(shape_to_path[shape]),
          glm::translate(glm::mat4(1.f), center_pos) * glm::toMat4(rotation)
              * glm::scale(glm::mat4(1.f), scale),
          scale, mass, SPECULAR),
      constraints() {
    member.get_body()->setFriction(friction);
}

std::string BuilderMember::get_name() { return member.get_name(); }
glm::vec3 BuilderMember::get_scale() {
    return bullet_to_glm(member.get_body()->getCollisionShape()->getLocalScaling());
}
float BuilderMember::get_friction() { return member.get_body()->getFriction(); }
float BuilderMember::get_mass() { return member.get_body()->getMass(); }
Item BuilderMember::get_item() { return member; }
ShapeKind BuilderMember::get_shape() { return shape; }

std::vector<std::shared_ptr<AbstractConstraint>> BuilderMember::get_children() {
    std::vector<std::shared_ptr<AbstractConstraint>> abstract_constraints;
    std::transform(
        constraints.begin(), constraints.end(), abstract_constraints.begin(),
        [](auto &c) { return static_cast<std::shared_ptr<AbstractConstraint>>(c); });
    return abstract_constraints;
}

std::vector<std::shared_ptr<BuilderConstraint>> BuilderMember::get_builder_children() {
    return constraints;
}

void BuilderMember::transform(
    std::optional<glm::vec3> new_center_pos, std::optional<glm::quat> new_rotation,
    std::optional<glm::vec3> new_scale) {

    const auto [curr_translation, curr_rotation, curr_scale] =
        decompose_model_matrix(member.model_matrix());

    const glm::mat4 rotation_matrix =
        glm::toMat4(new_center_pos.has_value() ? new_rotation.value() : curr_rotation);
    const glm::mat4 scale_matrix =
        glm::scale(glm::mat4(1.f), new_scale.has_value() ? new_scale.value() : curr_scale);
    const glm::mat4 translation_matrix = glm::translate(
        glm::mat4(1.f), new_center_pos.has_value() ? new_center_pos.value() : curr_translation);

    propagate_transform(translation_matrix * rotation_matrix * scale_matrix);
}

void BuilderMember::propagate_transform(glm::mat4 new_model_matrix) {
    member.get_body()->setWorldTransform(glm_to_bullet(new_model_matrix));

    const glm::mat4 new_model_matrix_without_scale = member.model_matrix_without_scale();

    std::queue<std::tuple<glm::mat4, std::shared_ptr<BuilderMember>>> member_queue;

    for (const auto &c: constraints)
        member_queue.emplace(new_model_matrix_without_scale, c->get_builder_member_child());

    while (!member_queue.empty()) {
        const auto &[curr_parent_model_matrix, child] = member_queue.front();
        member_queue.pop();

        child->get_item().get_body()->setWorldTransform(
            glm_to_bullet(child->member.model_matrix() * curr_parent_model_matrix));

        const glm::mat4 new_child_model_matrix = child->member.model_matrix_without_scale();

        for (const auto &c: child->constraints)
            member_queue.emplace(new_child_model_matrix, c->get_builder_member_child());
    }
}

void BuilderMember::add_fixed_constraint(
    const Item &parent, const glm::mat4 &attach_in_parent, const glm::mat4 &attach_in_child) {
    add_constraint<BuilderFixedConstraint>(
        parent, attach_in_parent, attach_in_child, shared_from_this());
}

void BuilderMember::add_hinge_constraint(
    const Item &parent, const glm::mat4 &frame_in_parent, const glm::mat4 &frame_in_child,
    float limit_degree_min, float limit_degree_max) {
    add_constraint<BuilderHingeConstraint>(
        parent, frame_in_parent, frame_in_child, limit_degree_min, limit_degree_max,
        shared_from_this());
}

template<typename ConstraintConstructor, typename... Args>
void BuilderMember::add_constraint(Args... args) {
    constraints.push_back(std::make_shared<ConstraintConstructor>(args...));
}
