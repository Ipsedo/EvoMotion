//
// Created by samuel on 19/01/25.
//

#ifndef EVO_MOTION_SKELETON_H
#define EVO_MOTION_SKELETON_H

#include <any>
#include <filesystem>
#include <map>
#include <memory>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "../controller.h"
#include "../item.h"
#include "../serializer.h"
#include "../shapes.h"
#include "../state.h"
#include "./constraint.h"
#include "./member.h"
#include "./muscle.h"

/*
 * Skeleton
 */

class Skeleton {
public:
    Skeleton(
        std::string robot_name, std::string root_name,
        const std::vector<std::shared_ptr<Member>> &members,
        const std::vector<std::shared_ptr<Constraint>> &constraints,
        const std::vector<std::shared_ptr<Muscle>> &muscles);

    explicit Skeleton(const std::shared_ptr<AbstractDeserializer> &deserializer);

    std::shared_ptr<Member> get_member(const std::string &name);

    std::vector<std::shared_ptr<AbstractItem>> get_items();

    std::vector<btRigidBody *> get_bodies();
    std::vector<btTypedConstraint *> get_constraints();

    std::vector<std::shared_ptr<Controller>> get_controllers() const;

    std::vector<std::shared_ptr<State>>
    get_states(const std::shared_ptr<RigidBodyItem> &floor, btDynamicsWorld *world);

    std::string get_root_name();
    std::string get_robot_name();

    virtual std::shared_ptr<AbstractSerializer>
    serialize(const std::shared_ptr<AbstractSerializer> &serializer);

protected:
    std::string robot_name;
    std::string root_name;
    std::vector<std::shared_ptr<Member>> members;
    std::vector<std::shared_ptr<Constraint>> constraints;
    std::vector<std::shared_ptr<Muscle>> muscles;
};

#endif//EVO_MOTION_SKELETON_H
