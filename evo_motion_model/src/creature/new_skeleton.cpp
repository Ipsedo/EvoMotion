//
// Created by samuel on 19/01/25.
//

#include <utility>

#include <evo_motion_model/constraint.h>
#include <evo_motion_model/member.h>
#include <evo_motion_model/new_skeleton.h>

#include "../controller/muscle_controller.h"
#include "../utils.h"

/*
 * Skeleton
 */

NewSkeleton::NewSkeleton(
    std::string robot_name, std::string root_name,
    const std::vector<std::shared_ptr<NewMember>> &members,
    const std::vector<std::shared_ptr<NewConstraint>> &constraints,
    const std::vector<std::shared_ptr<Muscle>> &muscles)
    : robot_name(std::move(robot_name)), root_name(std::move(root_name)), members(members),
      constraints(constraints), muscles(muscles) {}

NewSkeleton::NewSkeleton(const std::shared_ptr<AbstractDeserializer> &deserializer)
    : robot_name(deserializer->read_str("robot_name")),
      root_name(deserializer->read_str("root_name")),
      members(transform_vector<std::shared_ptr<AbstractDeserializer>, std::shared_ptr<NewMember>>(
          deserializer->read_array("members"),
          [](const std::shared_ptr<AbstractDeserializer> &d) {
              return std::make_shared<NewMember>(d);
          })),
      constraints(
          transform_vector<std::shared_ptr<AbstractDeserializer>, std::shared_ptr<NewConstraint>>(
              deserializer->read_array("constraints"),
              [this](const std::shared_ptr<AbstractDeserializer> &d)
                  -> std::shared_ptr<NewConstraint> {
                  if (d->read_str("type") == "fixed")
                      return std::make_shared<NewFixedConstraint>(
                          d, [this](const std::string &n) { return get_member(n); });
                  else if (d->read_str("type") == "hinge")
                      return std::make_shared<NewHingeConstraint>(
                          d, [this](const std::string &n) { return get_member(n); });
                  throw std::runtime_error("Unknown constraint type: " + d->read_str("type"));
              })),
      muscles(transform_vector<std::shared_ptr<AbstractDeserializer>, std::shared_ptr<Muscle>>(
          deserializer->read_array("muscles"),
          [this](const std::shared_ptr<AbstractDeserializer> &d) {
              return std::make_shared<Muscle>(
                  d, [this](const std::string &n) { return get_member(n); });
          })) {}

std::shared_ptr<NewMember> NewSkeleton::get_member(const std::string &name) {
    for (const auto &m: members)
        if (m->get_item().get_name() == name) return m;
    throw std::runtime_error("Member \"" + name + "\"not found");
}

std::vector<Item> NewSkeleton::get_items() {
    std::vector<Item> items;
    std::transform(members.begin(), members.end(), std::back_inserter(items), [](const auto &t) {
        return t->get_item();
    });

    for (const auto &m: muscles) {
        auto muscle_items = m->get_items();
        items.insert(items.end(), muscle_items.begin(), muscle_items.end());
    }

    return items;
}

std::vector<btTypedConstraint *> NewSkeleton::get_constraints() {
    std::vector<btTypedConstraint *> type_constraints;
    std::transform(
        constraints.begin(), constraints.end(), std::back_inserter(type_constraints),
        [](const auto &t) { return t->get_constraint(); });

    for (const auto &m: muscles) {
        auto muscle_constraints = m->get_constraints();
        type_constraints.insert(
            type_constraints.end(), muscle_constraints.begin(), muscle_constraints.end());
    }

    return type_constraints;
}

std::string NewSkeleton::get_root_name() { return root_name; }
std::string NewSkeleton::get_robot_name() { return robot_name; }

std::shared_ptr<AbstractSerializer<std::any>>
NewSkeleton::serialize(const std::shared_ptr<AbstractSerializer<std::any>> &serializer) {
    serializer->write_str("robot_name", robot_name);
    serializer->write_str("root_name", root_name);

    std::vector<std::shared_ptr<AbstractSerializer<std::any>>> serializer_members;
    std::transform(
        members.begin(), members.end(), std::back_inserter(serializer_members),
        [serializer](const auto &m) { return m->serialize(serializer); });
    serializer->write_array("members", serializer_members);

    std::vector<std::shared_ptr<AbstractSerializer<std::any>>> serializer_constraints;
    std::transform(
        constraints.begin(), constraints.end(), std::back_inserter(serializer_constraints),
        [serializer](const auto &c) { return c->serialize(serializer); });
    serializer->write_array("constraints", serializer_constraints);

    std::vector<std::shared_ptr<AbstractSerializer<std::any>>> serializer_muscles;
    std::transform(
        muscles.begin(), muscles.end(), std::back_inserter(serializer_muscles),
        [serializer](const auto &m) { return m->serialize(serializer); });
    serializer->write_array("muscles", serializer_muscles);

    return serializer;
}

std::vector<std::shared_ptr<Muscle>> NewSkeleton::get_muscles() { return muscles; }
