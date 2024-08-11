//
// Created by samuel on 01/04/24.
//

#ifndef EVO_MOTION_SKELETON_H
#define EVO_MOTION_SKELETON_H

#include <map>
#include <vector>

#include <btBulletDynamicsCommon.h>

#include <evo_motion_model/item.h>

#include "../converter.h"

class AbstractMember;

class AbstractConstraint {
public:
    virtual btTypedConstraint *get_constraint() = 0;

    virtual std::shared_ptr<AbstractMember> get_child() = 0;

    virtual ~AbstractConstraint();
};

class AbstractMember {
public:
    virtual Item get_item() = 0;

    virtual std::vector<std::shared_ptr<AbstractConstraint> > get_children() = 0;

    virtual ~AbstractMember();
};

class Skeleton {
public:
    Skeleton(std::string root_name, const std::shared_ptr<AbstractMember> &root_member);

    std::vector<Item> get_items();

    Item get_item(const std::string &name);

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

class JsonMember final : public AbstractMember {
public:
    JsonMember(const Item &parent, const nlohmann::json &json_member);

    JsonMember(
        const std::string &name, const glm::mat4 &parent_model_matrix,
        nlohmann::json json_member_input);

    Item get_item() override;

    std::vector<std::shared_ptr<AbstractConstraint> > get_children() override;

protected:
    nlohmann::json json_member;
    std::string name;
    glm::mat4 model_matrix;
    std::unordered_map<std::string, std::string> shape_to_path;
    std::shared_ptr<ObjShape> obj_shape;
    Item member;
};

class JsonHingeConstraint final : public AbstractConstraint {
public:
    JsonHingeConstraint(const Item &parent, const nlohmann::json &hinge);

    btTypedConstraint *get_constraint() override;

    std::shared_ptr<AbstractMember> get_child() override;

    ~JsonHingeConstraint() override;

private:
    btHingeConstraint *hinge_constraint;
    std::shared_ptr<JsonMember> child;
};

class JsonFixedConstraint final : public AbstractConstraint {
public:
    JsonFixedConstraint(const Item &parent, const nlohmann::json &fixed);

    btTypedConstraint *get_constraint() override;

    std::shared_ptr<AbstractMember> get_child() override;

    ~JsonFixedConstraint() override;

private:
    btFixedConstraint *fixed_constraint;
    std::shared_ptr<JsonMember> child;
};

class JsonSkeleton : public Skeleton {
public:
    JsonSkeleton(
        const std::string &json_path, const std::string &root_name, glm::mat4 model_matrix);
};

#endif//EVO_MOTION_SKELETON_H