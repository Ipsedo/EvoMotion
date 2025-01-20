//
// Created by samuel on 15/01/25.
//

#include <memory>

#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/json/json_skeleton.h>

#include "../converter.h"

/*
 * JSON stuff
 */

// skeleton

JsonSkeleton::JsonSkeleton(
    const std::string &json_path, const std::string &root_name, glm::mat4 model_matrix)
    : Skeleton(
          root_name,
          std::make_shared<JsonMember>(root_name, model_matrix, read_json(json_path)["skeleton"])) {
}

// member

JsonMember::JsonMember(
    const std::string &name, const glm::mat4 &parent_model_matrix, nlohmann::json json_member_input)
    : json_member(std::move(json_member_input)), model_matrix(parent_model_matrix),
      shape_to_path(
          {{"sphere", "./resources/obj/sphere.obj"},
           {"cube", "./resources/obj/cube.obj"},
           {"cylinder", "./resources/obj/cylinder.obj"},
           {"feet", "./resources/obj/feet.obj"}}),
      member(
          name + "_" + json_member["name"].get<std::string>(),
          std::make_shared<ObjShape>(shape_to_path[json_member["shape"].get<std::string>()]),
          model_matrix * json_transformation_to_model_matrix(json_member["transformation"]),
          json_vec3_to_glm_vec3(json_member["scale"]), json_member["mass"].get<float>(), SPECULAR) {

    auto option = json_member["option"];

    if (option.contains("friction"))
        member.get_body()->setFriction(option["friction"].get<float>());
    if (option.contains("ignore_collision") && option["ignore_collision"].get<bool>())
        member.get_body()->setCollisionFlags(
            member.get_body()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

JsonMember::JsonMember(const Item &parent, const nlohmann::json &json_member)
    : JsonMember(parent.get_name(), parent.model_matrix_without_scale(), json_member) {}

Item JsonMember::get_item() { return member; }

std::vector<std::shared_ptr<AbstractConstraint>> JsonMember::get_children() {
    std::vector<std::shared_ptr<AbstractConstraint>> children;

    std::map<
        std::string,
        std::function<std::shared_ptr<AbstractConstraint>(Item, const nlohmann::json &)>> const map{
        {"hinge", std::make_shared<JsonHingeConstraint, Item, const nlohmann::json &>},
        {"fixed", std::make_shared<JsonFixedConstraint, Item, const nlohmann::json &>}};

    for (const auto &child: json_member["children"])
        children.push_back(
            map.find(child["constraint_type"].get<std::string>())->second(member, child));

    return children;
}

// hinge

JsonHingeConstraint::JsonHingeConstraint(const Item &parent, const nlohmann::json &hinge)
    : child(std::make_shared<JsonMember>(parent, hinge["child_member"])),
      hinge_constraint(new btHingeConstraint(
          *parent.get_body(), *child->get_item().get_body(),
          glm_to_bullet(json_transformation_to_model_matrix(hinge["frame_in_parent"])),
          glm_to_bullet(json_transformation_to_model_matrix(hinge["frame_in_child"])))) {

    hinge_constraint->setLimit(
        M_PI * hinge["limit_degree"]["min"].get<float>() / 180.f,
        M_PI * hinge["limit_degree"]["max"].get<float>() / 180.f);

    hinge_constraint->setOverrideNumSolverIterations(
        hinge_constraint->getOverrideNumSolverIterations() * 8);

    parent.get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);
}

btTypedConstraint *JsonHingeConstraint::get_constraint() { return hinge_constraint; }

std::shared_ptr<AbstractMember> JsonHingeConstraint::get_child() { return child; }

JsonHingeConstraint::~JsonHingeConstraint() = default;

// fixed constraint

JsonFixedConstraint::JsonFixedConstraint(const Item &parent, const nlohmann::json &fixed)
    : child(std::make_shared<JsonMember>(parent, fixed["child_member"])),
      fixed_constraint(new btFixedConstraint(
          *parent.get_body(), *child->get_item().get_body(),
          glm_to_bullet(json_transformation_to_model_matrix(fixed["attach_in_parent"])),
          glm_to_bullet(json_transformation_to_model_matrix(fixed["attach_in_child"])))) {

    parent.get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);

    fixed_constraint->setOverrideNumSolverIterations(
        fixed_constraint->getOverrideNumSolverIterations() * 8);
}

btTypedConstraint *JsonFixedConstraint::get_constraint() { return fixed_constraint; }

std::shared_ptr<AbstractMember> JsonFixedConstraint::get_child() { return child; }

JsonFixedConstraint::~JsonFixedConstraint() = default;
