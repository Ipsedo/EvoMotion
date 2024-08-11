//
// Created by samuel on 01/04/24.
//

#include "./skeleton.h"

#include <fstream>
#include <queue>
#include <tuple>
#include <utility>

#include <glm/gtc/type_ptr.hpp>

/*
 * Abstract
 */

AbstractConstraint::~AbstractConstraint() = default;

AbstractMember::~AbstractMember() = default;

/*
 * Skeleton
 */

Skeleton::Skeleton(std::string root_name, const std::shared_ptr<AbstractMember> &root_member)
    : root_name(std::move(root_name)) {

    std::queue<std::shared_ptr<AbstractMember>> queue;
    queue.push(root_member);

    while (!queue.empty()) {
        const auto member = queue.front();
        queue.pop();

        items_map.emplace(member->get_item().get_name(), member->get_item());

        for (const auto &child: member->get_children()) {
            constraints.push_back(child->get_constraint());
            queue.push(child->get_child());
        }
    }
}

std::vector<Item> Skeleton::get_items() {
    std::vector<Item> items;
    std::transform(items_map.begin(), items_map.end(), std::back_inserter(items), [](auto t) {
        return std::get<1>(t);
    });
    return items;
}

std::vector<btTypedConstraint *> Skeleton::get_constraints() { return constraints; }

Item Skeleton::get_item(const std::string &name) { return items_map.find(name)->second; }

std::string Skeleton::get_root_name() { return root_name; }

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
    : json_member(std::move(json_member_input)), name(name), model_matrix(parent_model_matrix),
      shape_to_path(
          {{"sphere", "./resources/obj/sphere.obj"},
           {"cube", "./resources/obj/cube.obj"},
           {"cylinder", "./resources/obj/cylinder.obj"},
           {"feet", "./resources/obj/feet.obj"}}),
      member(
          name + "_" + json_member["name"].get<std::string>(),
          std::make_shared<ObjShape>(shape_to_path[json_member["shape"].get<std::string>()]),
          model_matrix * json_transformation_to_model_matrix(json_member["transformation"]),
          json_vec3_to_glm_vec3(json_member["scale"]), json_member["mass"].get<float>()) {

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

JsonHingeConstraint::JsonHingeConstraint(const Item &parent, const nlohmann::json &hinge) {

    child = std::make_shared<JsonMember>(parent, hinge["child_member"]);

    const btTransform frame_in_parent =
        glm_to_bullet(json_transformation_to_model_matrix(hinge["frame_in_parent"]));
    const btTransform frame_in_child =
        glm_to_bullet(json_transformation_to_model_matrix(hinge["frame_in_child"]));

    hinge_constraint = new btHingeConstraint(
        *parent.get_body(), *child->get_item().get_body(), frame_in_parent, frame_in_child);

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

JsonFixedConstraint::JsonFixedConstraint(const Item &parent, const nlohmann::json &fixed) {
    child = std::make_shared<JsonMember>(parent, fixed["child_member"]);

    btTransform parent_tr;
    parent_tr.setFromOpenGLMatrix(
        glm::value_ptr(json_transformation_to_model_matrix(fixed["attach_in_parent"])));
    btTransform sub_tr;
    sub_tr.setFromOpenGLMatrix(
        glm::value_ptr(json_transformation_to_model_matrix(fixed["attach_in_child"])));

    parent.get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);

    fixed_constraint =
        new btFixedConstraint(*parent.get_body(), *child->get_item().get_body(), parent_tr, sub_tr);

    fixed_constraint->setOverrideNumSolverIterations(
        fixed_constraint->getOverrideNumSolverIterations() * 8);
}

btTypedConstraint *JsonFixedConstraint::get_constraint() { return fixed_constraint; }

std::shared_ptr<AbstractMember> JsonFixedConstraint::get_child() { return child; }

JsonFixedConstraint::~JsonFixedConstraint() = default;