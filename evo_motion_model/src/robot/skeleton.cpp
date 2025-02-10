//
// Created by samuel on 19/01/25.
//

#include <utility>

#include <evo_motion_model/robot/constraint.h>
#include <evo_motion_model/robot/member.h>
#include <evo_motion_model/robot/skeleton.h>

#include "../controller/muscle_controller.h"
#include "../utils.h"
#include "./proprioception_state.h"

/*
 * Skeleton
 */

Skeleton::Skeleton(
    std::string robot_name, std::string root_name,
    const std::vector<std::shared_ptr<Member>> &members,
    const std::vector<std::shared_ptr<Constraint>> &constraints,
    const std::vector<std::shared_ptr<Muscle>> &muscles)
    : robot_name(std::move(robot_name)), root_name(std::move(root_name)), members(members),
      constraints(constraints), muscles(muscles) {}

Skeleton::Skeleton(const std::shared_ptr<AbstractDeserializer> &deserializer)
    : robot_name(deserializer->read_str("robot_name")),
      root_name(deserializer->read_str("root_name")),
      members(transform_vector<std::shared_ptr<AbstractDeserializer>, std::shared_ptr<Member>>(
          deserializer->read_array("members"),
          [](const std::shared_ptr<AbstractDeserializer> &d) {
              return std::make_shared<Member>(d);
          })),
      constraints(
          transform_vector<std::shared_ptr<AbstractDeserializer>, std::shared_ptr<Constraint>>(
              deserializer->read_array("constraints"),
              [this](
                  const std::shared_ptr<AbstractDeserializer> &d) -> std::shared_ptr<Constraint> {
                  if (d->read_str("type") == "fixed")
                      return std::make_shared<FixedConstraint>(
                          d, [this](const std::string &n) { return get_member(n); });
                  else if (d->read_str("type") == "hinge")
                      return std::make_shared<HingeConstraint>(
                          d, [this](const std::string &n) { return get_member(n); });
                  throw std::runtime_error("Unknown constraint type: " + d->read_str("type"));
              })),
      muscles(transform_vector<std::shared_ptr<AbstractDeserializer>, std::shared_ptr<Muscle>>(
          deserializer->read_array("muscles"),
          [this](const std::shared_ptr<AbstractDeserializer> &d) {
              return std::make_shared<Muscle>(
                  d, [this](const std::string &n) { return get_member(n); });
          })) {}

std::shared_ptr<Member> Skeleton::get_member(const std::string &name) {
    for (const auto &m: members)
        if (m->get_item()->get_name() == name) return m;
    throw std::runtime_error("Member \"" + name + "\"not found");
}

std::vector<std::shared_ptr<AbstractItem>> Skeleton::get_items() {
    std::vector<std::shared_ptr<AbstractItem>> items;
    std::transform(members.begin(), members.end(), std::back_inserter(items), [](const auto &t) {
        return t->get_item();
    });

    for (const auto &m: muscles) {
        auto muscle_items = m->get_items();
        items.insert(items.end(), muscle_items.begin(), muscle_items.end());
    }

    std::transform(
        constraints.begin(), constraints.end(), std::back_inserter(items),
        [](const auto &c) { return c->get_empty_item(); });

    return items;
}

std::vector<btTypedConstraint *> Skeleton::get_constraints() {
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

std::vector<btRigidBody *> Skeleton::get_bodies() {
    std::vector<btRigidBody *> bodies;

    std::transform(members.begin(), members.end(), std::back_inserter(bodies), [](const auto &m) {
        return m->get_item()->get_body();
    });

    for (const auto &m: muscles)
        for (const auto &b: m->get_bodies()) bodies.push_back(b);

    return bodies;
}

std::string Skeleton::get_root_name() { return root_name; }
std::string Skeleton::get_robot_name() { return robot_name; }

std::shared_ptr<AbstractSerializer>
Skeleton::serialize(const std::shared_ptr<AbstractSerializer> &serializer) {
    serializer->write_str("robot_name", robot_name);
    serializer->write_str("root_name", root_name);

    std::vector<std::shared_ptr<AbstractSerializer>> serializer_members;
    std::transform(
        members.begin(), members.end(), std::back_inserter(serializer_members),
        [serializer](const auto &m) { return m->serialize(serializer); });
    serializer->write_array("members", serializer_members);

    std::vector<std::shared_ptr<AbstractSerializer>> serializer_constraints;
    std::transform(
        constraints.begin(), constraints.end(), std::back_inserter(serializer_constraints),
        [serializer](const auto &c) { return c->serialize(serializer); });
    serializer->write_array("constraints", serializer_constraints);

    std::vector<std::shared_ptr<AbstractSerializer>> serializer_muscles;
    std::transform(
        muscles.begin(), muscles.end(), std::back_inserter(serializer_muscles),
        [serializer](const auto &m) { return m->serialize(serializer); });
    serializer->write_array("muscles", serializer_muscles);

    return serializer;
}

std::vector<std::shared_ptr<Controller>> Skeleton::get_controllers() const {
    int i = 0;
    return transform_vector<std::shared_ptr<Muscle>, std::shared_ptr<Controller>>(
        muscles, [&i](const auto &m) { return std::make_shared<MuscleController>(m, i++); });
}

std::vector<std::shared_ptr<State>>
Skeleton::get_states(const std::shared_ptr<RigidBodyItem> &floor, btDynamicsWorld *world) {
    std::vector<std::shared_ptr<State>> states;

    std::vector<std::shared_ptr<Member>> non_root_items;
    std::copy_if(
        members.begin(), members.end(), std::back_inserter(non_root_items),
        [this](const auto &i) { return i->get_item()->get_name() != get_root_name(); });

    const auto root_member = get_member(get_root_name());

    states.push_back(std::make_shared<RootMemberState>(root_member->get_item(), floor, world));

    for (const auto &member: non_root_items)
        states.push_back(std::make_shared<MemberState>(
            member->get_item(), root_member->get_item(), floor, world));

    for (const auto &muscle: muscles) states.push_back(std::make_shared<MuscleState>(muscle));

    return states;
}
