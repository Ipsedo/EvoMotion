//
// Created by samuel on 01/04/24.
//

#ifndef EVO_MOTION_SKELETON_H
#define EVO_MOTION_SKELETON_H

#include <vector>
#include <json/value.h>
#include <btBulletDynamicsCommon.h>

#include "./item.h"

class AbstractSkeleton {
public:
    virtual std::vector<std::shared_ptr<Item>> get_items() = 0;

    virtual std::vector<btTypedConstraint *> get_constraints() = 0;
};

class AbstractMember {
public:
    explicit AbstractMember(const std::shared_ptr<Item> &parent);

    virtual std::shared_ptr<Item> get_item() = 0;

    virtual btTypedConstraint *get_constraint() = 0;

    virtual std::vector<std::shared_ptr<AbstractMember>> get_children() = 0;

protected:
    std::shared_ptr<Item> parent;
};


/*
 * JSON stuff
 */

class JsonMember : public AbstractMember {
protected:

    const Json::Value &json_value;
    std::shared_ptr<Item> item;

public:
    JsonMember(const std::shared_ptr<Item> &parent, const Json::Value &member);

    std::shared_ptr<Item> get_item() override;

    std::vector<std::shared_ptr<AbstractMember>> get_children() override;
};

class JsonHingeMember : public JsonMember {
public:
    JsonHingeMember(const std::shared_ptr<Item> &parent, const Json::Value &hinge);

    btTypedConstraint *get_constraint() override;


private:
    btHingeConstraint *hinge_constraint;
};

class JsonFixedMember : public JsonMember {
public:
    JsonFixedMember(const std::shared_ptr<Item> &parent, const Json::Value &fixed);

    btTypedConstraint *get_constraint() override;
};


class JsonSkeleton : public AbstractSkeleton {
public:
    explicit JsonSkeleton(const std::string &json_path);

    std::vector<std::shared_ptr<Item>> get_items() override;

    std::vector<btTypedConstraint *> get_constraints() override;
};

#endif //EVO_MOTION_SKELETON_H
