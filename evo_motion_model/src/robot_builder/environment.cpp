//
// Created by samuel on 15/01/25.
//

#include <fstream>

#include <evo_motion_model/robot/builder.h>
#include <evo_motion_model/robot/skeleton.h>

#include "../converter.h"
#include "../json_serializer.h"

RobotBuilderEnvironment::RobotBuilderEnvironment()
    : Environment(1), root_name(), skeleton_graph(), members(), constraints() {}

bool RobotBuilderEnvironment::member_exists(const std::string &member_name) {
    return members.find(member_name) != members.end();
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
    const std::string &member_name, const std::optional<glm::vec3> new_pos,
    const std::optional<glm::quat> &new_rot, const std::optional<glm::vec3> new_scale,
    const std::optional<float> new_friction, const std::optional<bool> new_ignore_collision) {

    if (member_exists(member_name)) {
        std::set<std::string> updated_members;

        members[member_name]->update_item(
            new_pos, new_rot, new_scale, new_friction, new_ignore_collision);
        updated_members.insert(member_name);

        std::queue<std::string> queue;
        queue.push(member_name);

        while (!queue.empty()) {
            const auto &curr_member_name = queue.front();
            queue.pop();

            const auto curr_member = members[curr_member_name];

            curr_member->transform_item(new_pos, new_rot);

            updated_members.insert(curr_member_name);

            for (const auto &child_member_name: skeleton_graph[curr_member_name])
                if (updated_members.find(child_member_name) == updated_members.end())
                    queue.push(child_member_name);
        }

        return true;
    }
    return false;
}

void RobotBuilderEnvironment::save_robot(
    const std::filesystem::path &output_json_path, const std::string &robot_name) {
    std::vector<std::shared_ptr<Member>> members_vector;
    std::transform(
        members.begin(), members.end(), std::back_inserter(members_vector),
        [](const auto &pair) { return pair.second; });

    std::vector<std::shared_ptr<Constraint>> constraints_vector;
    std::transform(
        constraints.begin(), constraints.end(), std::back_inserter(constraints_vector),
        [](const auto &pair) { return pair.second; });

    Skeleton skeleton(robot_name, root_name, members_vector, constraints_vector, {});

    const auto json_serializer = std::make_shared<JsonSerializer>();
    skeleton.serialize(json_serializer);
    json_serializer->to_file(output_json_path);
}

void RobotBuilderEnvironment::load_robot(const std::filesystem::path &input_json_path) {}
