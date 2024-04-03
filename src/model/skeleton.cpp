//
// Created by samuel on 01/04/24.
//

#include <glm/gtc/matrix_transform.hpp>
#include <queue>
#include <fstream>
#include <iostream>
#include <tuple>
#include <glm/gtc/type_ptr.hpp>

#include "./skeleton.h"

Skeleton::Skeleton(const std::string &root_name, const std::shared_ptr<AbstractMember> &root_member)
    : root_name(root_name), constraints(), items_map() {

    std::queue<std::shared_ptr<AbstractMember>> queue;
    queue.push(root_member);

    while (!queue.empty()) {
        auto member = queue.front();
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
    std::transform(items_map.begin(), items_map.end(), std::back_inserter(items),
                   [](auto t) { return std::get<1>(t); });
    return items;
}

std::vector<btTypedConstraint *> Skeleton::get_constraints() {
    return constraints;
}

Item Skeleton::get_item(std::string name) {
    return items_map.find(name)->second;
}

std::string Skeleton::get_root_name() {
    return root_name;
}

/*
 * JSON stuff
 */

// skeleton

JsonSkeleton::JsonSkeleton(const std::string &json_path,
                           const std::string &root_name, glm::mat4 model_matrix)
    : Skeleton(root_name, std::make_shared<JsonMember>(root_name, model_matrix,
                                                       read_json(json_path)["skeleton"])) {

}

// member

Item JsonMember::build_item() {
    return {name + "_" + json_member["name"].asCString(),
            std::make_shared<ObjShape>(
                shape_to_path[json_member["shape"].asCString()]),
            model_matrix *
            json_transformation_to_model_matrix(json_member["transformation"]),
            json_vec3_to_glm_vec3(json_member["scale"]),
            json_member["mass"].asFloat()};
}

JsonMember::JsonMember(std::string name, glm::mat4 model_matrix,
                       Json::Value json_member) :
    json_member(std::move(json_member)), name(std::move(name)),
    model_matrix(model_matrix),
    shape_to_path({
                      {"sphere",   "./resources/obj/sphere.obj"},
                      {"cube",     "./resources/obj/cube.obj"},
                      {"cylinder", "./resources/obj/cylinder.obj"}
                  }),
    member(build_item()) {
}

JsonMember::JsonMember(Item parent, const Json::Value &member) :
    JsonMember(parent.get_name(),
               parent.model_matrix_without_scale(),
               member) {
}

Item JsonMember::get_item() {

    return member;
}

std::vector<std::shared_ptr<AbstractConstraint>> JsonMember::get_children() {
    std::vector<std::shared_ptr<AbstractConstraint>> children;

    std::map<std::string, std::function<std::shared_ptr<AbstractConstraint>(Item, const Json::Value &)>> map{
        {"hinge", std::make_shared<JsonHingeConstraint, Item, const Json::Value &>},
        {"fixed", std::make_shared<JsonFixedConstraint, Item, const Json::Value &>}
    };

    for (const auto &child: json_member["children"])
        children.push_back(map.find(child["constraint_type"].asCString())->second(member, child));

    return children;
}

// hinge

JsonHingeConstraint::JsonHingeConstraint(Item parent,
                                         const Json::Value &json_constraint) {

    btVector3 parent_axis = json_vec3_to_bt_vector3(
        json_constraint["parent_axis"]);
    btVector3 child_axis = json_vec3_to_bt_vector3(
        json_constraint["child_axis"]);
    btVector3 pos_in_parent = json_vec3_to_bt_vector3(
        json_constraint["member_attach"]);
    btVector3 pos_in_child = json_vec3_to_bt_vector3(
        json_constraint["child_attach"]);

    child = std::make_shared<JsonMember>(parent,
                                         json_constraint["child_member"]);

    hinge_constraint = new btHingeConstraint(
        *parent.get_body(),
        *child->get_item().get_body(),
        pos_in_parent,
        pos_in_child,
        parent_axis, child_axis
    );

    hinge_constraint->setLimit(json_constraint["limit_radian"]["min"].asFloat(),
                               json_constraint["limit_radian"]["max"].asFloat());

    parent.get_body()->setIgnoreCollisionCheck(child->get_item().get_body(),
                                               true);
}

btTypedConstraint *JsonHingeConstraint::get_constraint() {
    return hinge_constraint;
}

std::shared_ptr<AbstractMember> JsonHingeConstraint::get_child() {
    return child;
}
// fixed constraint

JsonFixedConstraint::JsonFixedConstraint(Item parent,
                                         const Json::Value &json_constraint) {
    child = std::make_shared<JsonMember>(parent,
                                         json_constraint["child_member"]);

    btTransform parent_tr;
    parent_tr.setFromOpenGLMatrix(
        glm::value_ptr(json_transformation_to_model_matrix(json_constraint["attach_in_parent"])));
    btTransform sub_tr;
    sub_tr.setFromOpenGLMatrix(glm::value_ptr(json_transformation_to_model_matrix(json_constraint["attach_in_child"])));

    parent.get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);

    fixed_constraint = new btFixedConstraint(
        *parent.get_body(),
        *child->get_item().get_body(),
        parent_tr,
        sub_tr
    );
}

btTypedConstraint *JsonFixedConstraint::get_constraint() {
    return fixed_constraint;
}

std::shared_ptr<AbstractMember> JsonFixedConstraint::get_child() {
    return child;
}
