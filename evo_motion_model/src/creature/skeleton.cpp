//
// Created by samuel on 01/04/24.
//

#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <tuple>
#include <utility>

#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/skeleton.h>

/*
 * Abstract
 */

AbstractConstraint::~AbstractConstraint() = default;

AbstractMember::~AbstractMember() = default;

/*
 * Skeleton
 */

Skeleton::Skeleton(std::string robot_name, const std::shared_ptr<AbstractMember> &root_member)
    : robot_name(std::move(robot_name)), root_name(root_member->get_item().get_name()) {

    std::queue<std::shared_ptr<AbstractMember>> queue;
    queue.push(root_member);

    while (!queue.empty()) {
        const auto member = queue.front();
        queue.pop();

        items_map.emplace(member->get_item().get_name(), member->get_item());

        for (const auto &child: member->get_children()) {
            constraints.push_back(child->get_constraint());
            queue.push(child->get_child());
        }
    }
}

std::vector<Item> Skeleton::get_items() {
    std::vector<Item> items;
    std::transform(items_map.begin(), items_map.end(), std::back_inserter(items), [](auto t) {
        return std::get<1>(t);
    });
    return items;
}

std::vector<btTypedConstraint *> Skeleton::get_constraints() { return constraints; }

Item Skeleton::get_item(const std::string &name) { return items_map.find(name)->second; }

std::string Skeleton::get_root_name() { return root_name; }

std::string Skeleton::get_robot_name() { return robot_name; }