//
// Created by samuel on 15/01/25.
//

#include <fstream>

#include <evo_motion_model/robot_builder.h>

#include "../converter.h"

RobotBuilderEnvironment::RobotBuilderEnvironment()
    : Environment(1), root_name(), skeleton_graph(), members(), constraints() {}

void RobotBuilderEnvironment::save_robot(const std::filesystem::path &output_json_path) {}

void RobotBuilderEnvironment::load_robot(const std::filesystem::path &input_json_path) {}

bool RobotBuilderEnvironment::member_exists(const std::string &member_name) {
    return std::any_of(members.begin(), members.end(), [&](const auto &member) {
        return member->get_item().get_name() == member_name;
    });
}

bool RobotBuilderEnvironment::set_root(const std::string &member_name) {
    if (member_exists(member_name)) {
        root_name = member_name;
        return true;
    }
    return false;
}

bool RobotBuilderEnvironment::add_member(
    const std::string &member_name, glm::vec3 center_pos, glm::quat rotation, glm::vec3 scale,
    float mass, float friction) {
    return !member_exists(member_name);
}

bool RobotBuilderEnvironment::update_member(
    const std::string &member_name, glm::vec3 center_pos, glm::quat rotation, glm::vec3 scale,
    float mass, float friction) {
    return member_exists(member_name);
}
