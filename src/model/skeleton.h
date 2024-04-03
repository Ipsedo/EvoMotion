//
// Created by samuel on 01/04/24.
//

#ifndef EVO_MOTION_SKELETON_H
#define EVO_MOTION_SKELETON_H

#include <vector>
#include <btBulletDynamicsCommon.h>
#include <map>

#include "./item.h"
#include "./converter.h"

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
    explicit Skeleton(const std::string &root_name, const std::shared_ptr<AbstractMember> &root_member);

    std::vector<Item> get_items();

    Item get_item(std::string name);

    std::vector<btTypedConstraint *> get_constraints();

    std::string get_root_name();

private:
    std::string root_name;
    std::vector<btTypedConstraint *> constraints;
    std::map<std::string, Item> items_map;
};

/*
 * JSON stuff
 */

class JsonMember : public AbstractMember {
public:
    JsonMember(Item parent, const Json::Value &json_member);

    JsonMember(std::string name, glm::mat4 model_matrix,
               Json::Value json_member);

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
    JsonFixedConstraint(Item parent, const Json::Value &fixed);

    btTypedConstraint *get_constraint() override;

    std::shared_ptr<AbstractMember> get_child() override;

private:
    btFixedConstraint *fixed_constraint;
    std::shared_ptr<JsonMember> child;
};


class JsonSkeleton : public Skeleton {
public:
    explicit JsonSkeleton(const std::string &json_path,
                          const std::string &root_name, glm::mat4 model_matrix);
};

#endif //EVO_MOTION_SKELETON_H
