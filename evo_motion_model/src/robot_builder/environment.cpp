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
    const std::string &member_name, ShapeKind shape_kind, glm::vec3 center_pos, glm::quat rotation,
    glm::vec3 scale, float mass, float friction) {
    if (member_exists(member_name)) return false;

    skeleton_graph[member_name] = std::vector<std::tuple<std::string, std::string>>();

    members.push_back(std::make_shared<BuilderMember>(
        member_name, shape_kind, center_pos, rotation, scale, mass, friction, false));

    return true;
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

            for (const auto &[_, child_member_name]: skeleton_graph[curr_member_name])
                if (updated_members.find(child_member_name) == updated_members.end())
                    queue.push(child_member_name);
        }

        return true;
    }
    return false;
}

bool RobotBuilderEnvironment::attach_fixed_constraint(
    const std::string &constraint_name, const std::string &parent_name,
    const std::string &child_name, const glm::vec3 &absolute_fixed_point) {
    if (constraint_exists(constraint_name) || !member_exists(parent_name)
        || !member_exists(child_name))
        return false;

    const auto parent_model_matrix =
        get_member(parent_name)->get_item().model_matrix_without_scale();
    const auto child_model_matrix = get_member(child_name)->get_item().model_matrix_without_scale();

    const auto absolute_frame = glm::translate(glm::mat4(1.f), absolute_fixed_point);
    const auto frame_in_parent = glm::inverse(parent_model_matrix) * absolute_frame;
    const auto frame_in_child = glm::inverse(child_model_matrix) * absolute_frame;

    skeleton_graph[parent_name].emplace_back(constraint_name, child_name);
    skeleton_graph[child_name].emplace_back(constraint_name, parent_name);

    constraints.push_back(std::make_shared<BuilderFixedConstraint>(
        constraint_name, get_member(parent_name), get_member(child_name), frame_in_parent,
        frame_in_child));

    return true;
}

bool RobotBuilderEnvironment::remove_constraint(const std::string &constraint_name) {
    if (!constraint_exists(constraint_name)) return false;

    for (auto [member, children]: skeleton_graph) {
        std::erase_if(children, [constraint_name](const auto &t) {
            const auto &[curr_constraint_name, _] = t;
            return curr_constraint_name == constraint_name;
        });
        skeleton_graph[member] = children;
    }

    std::erase_if(
        constraints, [constraint_name](const auto &c) { return c->get_name() == constraint_name; });

    return true;
}

bool RobotBuilderEnvironment::member_exists(const std::string &member_name) {
    return exists_part(member_name, members);
}

bool RobotBuilderEnvironment::constraint_exists(const std::string &constraint_name) {
    return exists_part(constraint_name, constraints);
}

bool RobotBuilderEnvironment::muscle_exists(const std::string &muscle_name) {
    return exists_part(muscle_name, muscles);
}

template<typename Part>
bool RobotBuilderEnvironment::exists_part(
    const std::string &name, std::vector<std::shared_ptr<Part>> vec) {
    return std::any_of(
        vec.begin(), vec.end(), [name](const auto &p) { return p->get_name() == name; });
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
std::shared_ptr<Part>
RobotBuilderEnvironment::get_part(const std::string &name, std::vector<std::shared_ptr<Part>> vec) {
    for (const auto &o: vec)
        if (o->get_name() == name) return o;
    throw std::runtime_error("\"" + name + "\" not found");
}

void RobotBuilderEnvironment::save_robot(
    const std::filesystem::path &output_json_path, const std::string &robot_name) {

    std::vector<std::shared_ptr<Member>> members_vector(members.begin(), members.end());
    std::vector<std::shared_ptr<Constraint>> constraints_vector(
        constraints.begin(), constraints.end());
    std::vector<std::shared_ptr<Muscle>> muscles_vector(muscles.begin(), muscles.end());

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
        std::back_inserter(members), [](const auto &s) -> std::shared_ptr<BuilderMember> {
            return std::make_shared<BuilderMember>(s);
        });

    std::transform(
        json_constraints_deserializer.begin(), json_constraints_deserializer.end(),
        std::back_inserter(constraints),
        [this](const auto &s) -> std::shared_ptr<BuilderConstraint> {
            std::shared_ptr<BuilderConstraint> c;
            if (s->read_str("type") == "hinge")
                c = std::make_shared<BuilderHingeConstraint>(
                    s, [this](const auto &n) { return get_member(n); });
            else if (s->read_str("type") == "fixed")
                c = std::make_shared<BuilderFixedConstraint>(
                    s, [this](const auto &n) { return get_member(n); });
            else
                throw std::runtime_error("Unknown constraint type \"" + s->read_str("type") + "\"");

            return c;
        });

    std::transform(
        json_muscles_deserializer.begin(), json_muscles_deserializer.end(),
        std::back_inserter(muscles), [this](const auto &s) -> std::shared_ptr<BuilderMuscle> {
            return std::make_shared<BuilderMuscle>(
                s, [this](const auto &n) { return get_member(n); });
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
