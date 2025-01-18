//
// Created by samuel on 01/04/24.
//

#ifndef EVO_MOTION_SKELETON_H
#define EVO_MOTION_SKELETON_H

#include <map>
#include <vector>

#include <btBulletDynamicsCommon.h>

#include <evo_motion_model/item.h>

/*
 * Enum
 */

enum ShapeKind { CUBE, SPHERE, CYLINDER, FEET };

enum ConstraintKind { FIXED, HINGE, CONE };

/*
 * Abstract
 */

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

    virtual std::vector<std::shared_ptr<AbstractConstraint>> get_children() = 0;

    virtual ~AbstractMember();
};

class Skeleton {
public:
    Skeleton(std::string robot_name, const std::shared_ptr<AbstractMember> &root_member);

    std::vector<Item> get_items();

    Item get_item(const std::string &name);

    std::vector<btTypedConstraint *> get_constraints();

    std::string get_root_name();
    std::string get_robot_name();

private:
    std::string robot_name;
    std::string root_name;
    std::vector<btTypedConstraint *> constraints;
    std::unordered_map<std::string, Item> items_map;
};

#endif//EVO_MOTION_SKELETON_H