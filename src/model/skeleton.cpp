//
// Created by samuel on 01/04/24.
//

#include <glm/gtc/matrix_transform.hpp>
#include <queue>
#include <json/json.h>
#include <fstream>
#include <iostream>
#include <utility>

#include "./skeleton.h"

Skeleton::Skeleton(const std::shared_ptr<AbstractMember> &root_member) : items(), constraints() {

    std::queue<std::shared_ptr<AbstractMember>> queue;
    queue.push(root_member);

    while (!queue.empty()) {
        auto member = queue.front();
        queue.pop();

        items.push_back(member->get_item());
        for (const auto &child: member->get_children()) {
            constraints.push_back(child->get_constraint());
            queue.push(child->get_child());
        }
    }
}

std::vector<Item> Skeleton::get_items() {
    return items;
}

std::vector<btTypedConstraint *> Skeleton::get_constraints() {
    return constraints;
}

/*
 * JSON stuff
 */

// transformation

glm::mat4 json_transformation_to_model_matrix(Json::Value transformation) {

    Json::Value rotation = transformation["rotation"];
    Json::Value translation = transformation["translation"];

    glm::vec3 position(
        translation["x"].asFloat(),
        translation["y"].asFloat(),
        translation["z"].asFloat()
    );

    glm::vec3 rotation_point(
        rotation["point_x"].asFloat(),
        rotation["point_y"].asFloat(),
        rotation["point_z"].asFloat()
    );

    glm::vec3 rotation_axis(
        rotation["axis_x"].asFloat(),
        rotation["axis_y"].asFloat(),
        rotation["axis_z"].asFloat()
    );

    float angle_radian = rotation["angle_radian"].asFloat();

    glm::mat4 translation_to_origin = glm::translate(glm::mat4(1.0f), -rotation_point);
    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), angle_radian, rotation_axis);
    glm::mat4 translation_back = glm::translate(glm::mat4(1.0f), rotation_point);
    glm::mat4 translation_to_position = glm::translate(glm::mat4(1.0f), position);

    return translation_to_position * translation_back * rotation_matrix * translation_to_origin;
}


Json::Value read_json(const std::string &json_path) {
    std::ifstream stream(json_path, std::ios::in);

    Json::Value skeleton_json;

    stream >> skeleton_json;

    return skeleton_json;
}

// skeleton

JsonSkeleton::JsonSkeleton(const std::string &json_path, const std::string &root_name, glm::mat4 model_matrix)
    : Skeleton(std::make_shared<JsonMember>(root_name, model_matrix,
                                            read_json(json_path))) {

}

// member

Item JsonMember::build_item() {
    return {name + "_" + json_member["name"].asCString(),
            std::make_shared<ObjShape>(shape_to_path[json_member["shape"].asCString()]),
            model_matrix * json_transformation_to_model_matrix(json_member["transformation"]),
            glm::vec3(
                json_member["scale_x"].asFloat(),
                json_member["scale_y"].asFloat(),
                json_member["scale_z"].asFloat()),
            json_member["mass"].asFloat()};
}

JsonMember::JsonMember(std::string name, glm::mat4 model_matrix, Json::Value json_member) :
    json_member(std::move(json_member)), name(std::move(name)), model_matrix(model_matrix),
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

    for (const auto &child: json_member["children"]) {
        std::string constraint_type = child["constraint_type"].asCString();
        if (constraint_type == "hinge")
            children.push_back(std::make_shared<JsonHingeConstraint>(member, child));
        else if (constraint_type == "fixed")
            children.push_back(std::make_shared<JsonFixedConstraint>(member, child));
    }
    return children;
}

// hinge

JsonHingeConstraint::JsonHingeConstraint(Item parent, const Json::Value &json_constraint) {

    btVector3 parent_axis(
        json_constraint["parent_axis_x"].asFloat(),
        json_constraint["parent_axis_y"].asFloat(),
        json_constraint["parent_axis_z"].asFloat());

    btVector3 child_axis(
        json_constraint["child_axis_x"].asFloat(),
        json_constraint["child_axis_y"].asFloat(),
        json_constraint["child_axis_z"].asFloat());

    glm::vec3 pos_in_parent(
        json_constraint["member_attach_x"].asFloat(),
        json_constraint["member_attach_y"].asFloat(),
        json_constraint["member_attach_z"].asFloat()
    );
    glm::vec3 pos_in_child(
        json_constraint["child_attach_x"].asFloat(),
        json_constraint["child_attach_y"].asFloat(),
        json_constraint["child_attach_z"].asFloat()
    );

    child = std::make_shared<JsonMember>(parent, json_constraint["child_member"]);

    hinge_constraint = new btHingeConstraint(
        *parent.get_body(),
        *child->get_item().get_body(),
        btVector3(pos_in_parent.x, pos_in_parent.y, pos_in_parent.z),
        btVector3(pos_in_child.x, pos_in_child.y, pos_in_child.z),
        parent_axis, child_axis
    );

    parent.get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);
}

btTypedConstraint *JsonHingeConstraint::get_constraint() {
    return hinge_constraint;
}

std::shared_ptr<AbstractMember> JsonHingeConstraint::get_child() {
    return child;
}
// fixed constraint

JsonFixedConstraint::JsonFixedConstraint(const Item &parent, const Json::Value &json_constraint) {
    child = std::make_shared<JsonMember>(parent, json_constraint["member_info"]);
}

btTypedConstraint *JsonFixedConstraint::get_constraint() {
    return fixed_constraint;
}

std::shared_ptr<AbstractMember> JsonFixedConstraint::get_child() {
    return child;
}
