//
// Created by samuel on 19/01/25.
//

#ifndef EVO_MOTION_NEW_SKELETON_H
#define EVO_MOTION_NEW_SKELETON_H

#include <any>
#include <filesystem>
#include <map>
#include <memory>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "./constraint.h"
#include "./controller.h"
#include "./item.h"
#include "./member.h"
#include "./muscle.h"
#include "./serializer.h"
#include "./shapes.h"
#include "./state.h"

/*
 * Skeleton
 */

class NewSkeleton {
public:
    NewSkeleton(
        std::string robot_name, std::string root_name,
        const std::vector<std::shared_ptr<NewMember>> &members,
        const std::vector<std::shared_ptr<NewConstraint>> &constraints,
        const std::vector<std::shared_ptr<Muscle>> &muscles);

    explicit NewSkeleton(const std::shared_ptr<AbstractDeserializer> &deserializer);

    std::shared_ptr<NewMember> get_member(const std::string &name);

    std::vector<Item> get_items();
    std::vector<btTypedConstraint *> get_constraints();

    std::vector<std::shared_ptr<Controller>> get_controllers();

    std::vector<std::shared_ptr<State>> get_states(const Item &floor, btDynamicsWorld *world);

    std::string get_root_name();
    std::string get_robot_name();

    virtual std::shared_ptr<AbstractSerializer<std::any>>
    serialize(const std::shared_ptr<AbstractSerializer<std::any>> &serializer);

protected:
    std::string robot_name;
    std::string root_name;
    std::vector<std::shared_ptr<NewMember>> members;
    std::vector<std::shared_ptr<NewConstraint>> constraints;
    std::vector<std::shared_ptr<Muscle>> muscles;
};

#endif//EVO_MOTION_NEW_SKELETON_H
