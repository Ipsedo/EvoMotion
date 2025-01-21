//
// Created by samuel on 15/01/25.
//

#include <fstream>

#include <evo_motion_model/robot/builder.h>
#include <evo_motion_model/robot/skeleton.h>

#include "../json_serializer.h"

RobotBuilderEnvironment::RobotBuilderEnvironment()
    : Environment(1), root_name(), skeleton_graph(), members(), constraints(), muscles() {}

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

        get_member(member_name)
            ->update_item(new_pos, new_rot, new_scale, new_friction, new_ignore_collision);
        updated_members.insert(member_name);

        std::queue<std::string> queue;
        queue.push(member_name);

        while (!queue.empty()) {
            const auto &curr_member_name = queue.front();
            queue.pop();

            const auto curr_member = get_member(curr_member_name);

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

bool RobotBuilderEnvironment::member_exists(const std::string &member_name) {
    return std::any_of(members.begin(), members.end(), [member_name](const auto &t) {
        return std::get<0>(t) == member_name;
    });
}

bool RobotBuilderEnvironment::constraint_exists(const std::string &constraint_name) {
    return std::any_of(constraints.begin(), constraints.end(), [constraint_name](const auto &t) {
        return std::get<0>(t) == constraint_name;
    });
}

bool RobotBuilderEnvironment::muscle_exists(const std::string &muscle_name) {
    return std::any_of(muscles.begin(), muscles.end(), [muscle_name](const auto &t) {
        return std::get<0>(t) == muscle_name;
    });
}

std::shared_ptr<BuilderMember> RobotBuilderEnvironment::get_member(const std::string &member_name) {
    return RobotBuilderEnvironment::get_part(member_name, members);
}

std::shared_ptr<BuilderConstraint>
RobotBuilderEnvironment::get_constraint(const std::string &constraint_name) {
    return RobotBuilderEnvironment::get_part(constraint_name, constraints);
}

std::shared_ptr<BuilderMuscle> RobotBuilderEnvironment::get_muscle(const std::string &muscle_name) {
    return RobotBuilderEnvironment::get_part(muscle_name, muscles);
}

template<typename Part>
std::shared_ptr<Part> RobotBuilderEnvironment::get_part(
    const std::string &name, std::vector<std::tuple<std::string, std::shared_ptr<Part>>> vec) {
    for (const auto &[n, o]: vec)
        if (n == name) return o;
    throw std::runtime_error("\"" + name + "\" not found");
}

void RobotBuilderEnvironment::save_robot(
    const std::filesystem::path &output_json_path, const std::string &robot_name) {
    std::vector<std::shared_ptr<Member>> members_vector;
    std::transform(
        members.begin(), members.end(), std::back_inserter(members_vector),
        [](const auto &pair) { return std::get<1>(pair); });

    std::vector<std::shared_ptr<Constraint>> constraints_vector;
    std::transform(
        constraints.begin(), constraints.end(), std::back_inserter(constraints_vector),
        [](const auto &pair) { return std::get<1>(pair); });

    std::vector<std::shared_ptr<Muscle>> muscles_vector;
    std::transform(
        muscles.begin(), muscles.end(), std::back_inserter(muscles_vector),
        [](const auto &pair) { return std::get<1>(pair); });

    Skeleton skeleton(robot_name, root_name, members_vector, constraints_vector, muscles_vector);

    const auto json_serializer = std::make_shared<JsonSerializer>();
    skeleton.serialize(json_serializer);
    json_serializer->to_file(output_json_path);
}

void RobotBuilderEnvironment::load_robot(const std::filesystem::path &input_json_path) {
    const auto json_deserializer = std::make_shared<JsonDeserializer>(input_json_path);

    const auto json_members_deserializer = json_deserializer->read_array("members");
    const auto json_constraints_deserializer = json_deserializer->read_array("constraints");
    const auto json_muscles_deserializer = json_deserializer->read_array("muscles");

    root_name = json_deserializer->read_str("root_name");

    std::transform(
        json_members_deserializer.begin(), json_members_deserializer.end(),
        std::back_inserter(members),
        [](const auto &s) -> std::pair<std::string, std::shared_ptr<BuilderMember>> {
            const auto m = std::make_shared<BuilderMember>(s);
            return {m->get_item().get_name(), m};
        });

    std::transform(
        json_constraints_deserializer.begin(), json_constraints_deserializer.end(),
        std::back_inserter(constraints),
        [this](const auto &s) -> std::pair<std::string, std::shared_ptr<BuilderConstraint>> {
            std::shared_ptr<BuilderConstraint> c;
            if (s->read_str("type") == "hinge")
                c = std::make_shared<BuilderHingeConstraint>(
                    s, [this](const auto &n) { return get_member(n); });
            else if (s->read_str("type") == "fixed")
                c = std::make_shared<BuilderFixedConstraint>(
                    s, [this](const auto &n) { return get_member(n); });
            else
                throw std::runtime_error("Unknown constraint type \"" + s->read_str("type") + "\"");

            return {c->get_name(), c};
        });

    std::transform(
        json_muscles_deserializer.begin(), json_muscles_deserializer.end(),
        std::back_inserter(muscles),
        [this](const auto &s) -> std::pair<std::string, std::shared_ptr<BuilderMuscle>> {
            const auto m =
                std::make_shared<BuilderMuscle>(s, [this](const auto &n) { return get_member(n); });
            return {m->get_name(), m};
        });
}

/*
 * Environment methods
 */

std::vector<Item> RobotBuilderEnvironment::get_items() { return std::vector<Item>(); }
std::vector<std::shared_ptr<Controller>> RobotBuilderEnvironment::get_controllers() {
    return std::vector<std::shared_ptr<Controller>>();
}
std::vector<int64_t> RobotBuilderEnvironment::get_state_space() { return std::vector<int64_t>(); }
std::vector<int64_t> RobotBuilderEnvironment::get_action_space() { return std::vector<int64_t>(); }
std::optional<Item> RobotBuilderEnvironment::get_camera_track_item() {
    return std::optional<Item>();
}
step RobotBuilderEnvironment::compute_step() { return step(); }
void RobotBuilderEnvironment::reset_engine() {}
