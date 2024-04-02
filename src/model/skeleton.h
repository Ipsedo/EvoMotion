//
// Created by samuel on 01/04/24.
//

#ifndef EVO_MOTION_SKELETON_H
#define EVO_MOTION_SKELETON_H

#include <vector>
#include <json/value.h>
#include <btBulletDynamicsCommon.h>

#include "./item.h"

class AbstractMember;

class AbstractConstraint {
public:

    virtual btTypedConstraint *get_constraint() = 0;

    virtual std::shared_ptr<AbstractMember> get_child() = 0;
};

class AbstractMember {
public:

    virtual Item get_item() = 0;

    virtual std::vector<std::shared_ptr<AbstractConstraint>> get_children() = 0;
};


class Skeleton {
public:
    explicit Skeleton(const std::shared_ptr<AbstractMember> &root_member);

    std::vector<Item> get_items();

    std::vector<btTypedConstraint *> get_constraints();

private:
    std::vector<Item> items;
    std::vector<btTypedConstraint *> constraints;
};

/*
 * JSON stuff
 */

glm::mat4 json_transformation_to_model_matrix(Json::Value transformation);

class JsonMember : public AbstractMember {
public:
    JsonMember(Item parent, const Json::Value &json_member);

    JsonMember(std::string name, glm::mat4 model_matrix, Json::Value json_member);

    Item get_item() override;

    std::vector<std::shared_ptr<AbstractConstraint>> get_children() override;

protected:
    Json::Value json_member;
    std::string name;
    glm::mat4 model_matrix;
    std::unordered_map<std::string, std::string> shape_to_path;
    std::shared_ptr<ObjShape> obj_shape;
    Item member;
private:
    Item build_item();
};

class JsonHingeConstraint : public AbstractConstraint {
public:
    JsonHingeConstraint(Item parent, const Json::Value &hinge);

    btTypedConstraint *get_constraint() override;

    std::shared_ptr<AbstractMember> get_child() override;


private:
    btHingeConstraint *hinge_constraint;
    std::shared_ptr<JsonMember> child;
};

class JsonFixedConstraint : public AbstractConstraint {
public:
    JsonFixedConstraint(const Item &parent, const Json::Value &fixed);

    btTypedConstraint *get_constraint() override;

    std::shared_ptr<AbstractMember> get_child() override;

private:
    btFixedConstraint *fixed_constraint;
    std::shared_ptr<JsonMember> child;
};


class JsonSkeleton : public Skeleton {
public:
    explicit JsonSkeleton(const std::string &json_path, const std::string &root_name, glm::mat4 model_matrix);
};

#endif //EVO_MOTION_SKELETON_H
