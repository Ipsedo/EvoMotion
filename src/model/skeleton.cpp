//
// Created by samuel on 01/04/24.
//

#include "./skeleton.h"

AbstractMember::AbstractMember(const std::shared_ptr<Item> &parent) : parent(parent) {

}

/*
 * JSON stuff
 */


// skeleton

JsonSkeleton::JsonSkeleton(const std::string &json_path) {

}

std::vector<std::shared_ptr<Item>> JsonSkeleton::get_items() {
    return {};
}

std::vector<btTypedConstraint *> JsonSkeleton::get_constraints() {
    return {};
}

// abstract

JsonMember::JsonMember(const std::shared_ptr<Item> &parent, const Json::Value &member) : AbstractMember(parent),
                                                                                         json_value(member) {

    Json::Value member_info = json_value["member_info"];

    std::string name = json_value["name"].asString();
    std::string shape_name = member_info["shape"].asString();
    glm::vec3 scale(member_info["scale_x"].asFloat(), member_info["scale_y"].asFloat(),
                    member_info["scale_z"].asFloat());
    float mass = member_info["mass"].asFloat();

    std::unordered_map<std::string, std::string> shape_to_path = {
        {"sphere",   "./resources/obj/sphere.obj"},
        {"cube",     "./resources/obj/cube.obj"},
        {"cylinder", "./resources/obj/cylinder.obj"}
    };

    auto shape = std::make_shared<ObjShape>(shape_to_path[shape_name]);

    glm::mat4 model_in_parent(1); // TODO model matrix as JSON
    glm::mat4 model_matrix = parent->model_matrix_without_scale() * model_in_parent;

    item = std::make_shared<Item>(parent->get_name() + "_" + name, shape, model_matrix, scale, mass);
}

std::shared_ptr<Item> JsonMember::get_item() {
    return item;
}

std::vector<std::shared_ptr<AbstractMember>> JsonMember::get_children() {
    std::vector<std::shared_ptr<AbstractMember>> children;
    for (const auto &child: json_value["children"])
        children.push_back(std::make_shared<JsonHingeMember>(item, child)); // TODO constraint factory
    return children;
}

// hinge

JsonHingeMember::JsonHingeMember(const std::shared_ptr<Item> &parent, const Json::Value &hinge) : JsonMember(parent,
                                                                                                             hinge) {
    btVector3 axis(
        json_value["axis_x"].asFloat(),
        json_value["axis_y"].asFloat(),
        json_value["axis_z"].asFloat());

    // TODO attach in parent and sub
    glm::mat4 attach_in_parent(1);
    glm::mat4 attach_in_sub(1);

    glm::vec3 pos_in_parent = glm::vec3(attach_in_parent * glm::vec4(0, 0, 0, 1));
    glm::vec3 pos_in_sub = glm::vec3(attach_in_sub * glm::vec4(0, 0, 0, 1));

    hinge_constraint = new btHingeConstraint(
        *parent->get_body(),
        *item->get_body(),
        btVector3(pos_in_parent.x, pos_in_parent.y, pos_in_parent.z),
        btVector3(pos_in_sub.x, pos_in_sub.y, pos_in_sub.z),
        axis, axis
    );

    parent->get_body()->setIgnoreCollisionCheck(item->get_body(), true);
}

btTypedConstraint *JsonHingeMember::get_constraint() {
    return hinge_constraint;
}

// fixed constraint

btTypedConstraint *JsonFixedMember::get_constraint() {
    return nullptr;
}
