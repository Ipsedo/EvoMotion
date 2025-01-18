//
// Created by samuel on 15/01/25.
//

#include <fstream>

#include <evo_motion_model/robot_builder.h>

#include "../converter.h"

RobotBuilderEnvironment::RobotBuilderEnvironment()
    : Environment(1), skeleton_tree(), root(nullptr), members_not_attached(),
      shape_to_name({{SPHERE, "sphere"}, {CUBE, "cube"}, {CYLINDER, "cylinder"}, {FEET, "feet"}}) {}

bool RobotBuilderEnvironment::add_root(
    const std::string &name, glm::vec3 center_pos, glm::vec3 scale, float mass, float friction) {
    return false;
}

bool RobotBuilderEnvironment::add_item(
    const std::string &name, glm::vec3 center_pos, glm::vec3 scale, float mass, float friction) {
    return false;
}

bool RobotBuilderEnvironment::attach_item_fixed_constraint(
    const std::string &name, const std::string &parent) {
    return false;
}
bool RobotBuilderEnvironment::attach_item_hinge_constraint(
    const std::string &name, const std::string &parent) {
    return false;
}
bool RobotBuilderEnvironment::attach_item_cone_constraint(
    const std::string &name, const std::string &parent) {
    return false;
}

void RobotBuilderEnvironment::robot_to_json(
    const std::filesystem::path &output_path, const std::string &robot_name) {

    nlohmann::json json;

    std::queue<
        std::tuple<std::vector<nlohmann::json>, glm::mat4, std::shared_ptr<BuilderConstraint>>>
        queue;

    json["skeleton"] = {
        {"name", root->get_name()},
        {"mass", root->get_mass()},
        {"scale", vec3_to_json(root->get_scale())},
        {"shape", shape_to_name[root->get_shape()]},
        {"transformation", model_matrix_to_json_transformation(root->get_item().model_matrix())},
        {"children", std::vector<nlohmann::json>()}};

    const glm::mat4 root_model_matrix_without_scale = root->get_item().model_matrix_without_scale();

    for (const auto &c: root->get_builder_children())
        queue.emplace(json["skeleton"]["children"], root_model_matrix_without_scale, c);

    while (!queue.empty()) {
        auto [array, parent_model_matrix_without_scale, constraint] = queue.front();
        queue.pop();

        nlohmann::json member_constraint_json;
        // TODO dump constraint

        const auto member = constraint->get_builder_member_child();

        // relative model matrix, member transformation in parent referential
        const glm::mat4 relative_member_model_matrix =
            glm::inverse(parent_model_matrix_without_scale) * member->get_item().model_matrix();

        nlohmann::json member_json{
            {"name", member->get_name()},
            {"mass", member->get_mass()},
            {"scale", vec3_to_json(member->get_scale())},
            {"shape", shape_to_name[member->get_shape()]},
            {"transformation", model_matrix_to_json_transformation(relative_member_model_matrix)},
            {"children", std::vector<nlohmann::json>()}};

        member_constraint_json["child_member"] = member_json;

        array.push_back(member_constraint_json);

        // absolute referential
        const glm::mat4 absolute_member_model_matrix =
            member->get_item().model_matrix_without_scale();

        for (const auto &c: member->get_builder_children())
            queue.emplace(member_json["children"], absolute_member_model_matrix, c);
    }

    std::vector<nlohmann::json> muscles_json;
    json["muscles"] = muscles_json;

    const std::string json_string = json.dump();

    std::ofstream json_file(output_path / (robot_name + ".json"));
    json_file << json_string;
    json_file.close();
}

void RobotBuilderEnvironment::json_to_robot(
    const std::filesystem::path &json_path, const std::string &robot_name) {
    nlohmann::json json = read_json(json_path);

    nlohmann::json skeleton_json = json["skeleton"];

    std::map<std::string, ShapeKind> name_to_shape;
    std::transform(
        shape_to_name.begin(), shape_to_name.end(),
        std::inserter(name_to_shape, name_to_shape.begin()),
        [](const auto &t) { return std::pair<std::string, ShapeKind>(t.second, t.first); });

    glm::mat4 root_model_matrix =
        json_transformation_to_model_matrix(skeleton_json["transformation"]);

    auto [root_translation, root_rotation, root_scale] = decompose_model_matrix(root_model_matrix);

    std::shared_ptr<BuilderMember> root_member = std::make_shared<BuilderMember>(
        skeleton_json["name"].get<std::string>(),
        name_to_shape[skeleton_json["shape"].get<std::string>()], root_translation, root_rotation,
        root_scale, skeleton_json["mass"].get<float>(), 0.1f);

    std::queue<std::tuple<std::shared_ptr<BuilderMember>, glm::mat4, nlohmann::json>> queue;
    queue.emplace(root_member, root_model_matrix, skeleton_json);

    while (!queue.empty()) {
        auto [member, member_model_matrix, json_member] = queue.front();
        queue.pop();

        for (const auto &constraint: json_member["children"]) {

            // TODO parse constraint

            const auto child = constraint["child_member"];

            glm::mat4 child_model_matrix =
                json_transformation_to_model_matrix(child["transformation"]) * member_model_matrix;

            auto [child_translate, child_rotation, child_scale] =
                decompose_model_matrix(child_model_matrix);

            std::shared_ptr<BuilderMember> child_member = std::make_shared<BuilderMember>(
                child["name"].get<std::string>(), name_to_shape[child["shape"].get<std::string>()],
                child_translate, child_rotation, child_scale, child["mass"].get<float>(), 0.1f);

            queue.emplace(member, child_model_matrix, child);
        }
    }

    nlohmann::json muscles_json = json["muscles"];
}

bool RobotBuilderEnvironment::member_exists(const std::string &name) {
    return skeleton_tree.find(name) != skeleton_tree.end();
}

bool RobotBuilderEnvironment::will_produce_cycle(
    const std::string &parent, const std::string &member) {
    std::map<std::string, std::vector<std::string>> potential_skeleton_tree;
    potential_skeleton_tree.insert(skeleton_tree.begin(), skeleton_tree.end());

    potential_skeleton_tree[parent].push_back(member);

    std::unordered_set<std::string> visited;
    std::queue<std::string> queue;

    queue.push(parent);

    while (!queue.empty()) {
        std::string curr = queue.front();
        queue.pop();

        if (visited.find(curr) != visited.end()) return true;

        visited.insert(curr);

        for (const auto &child: potential_skeleton_tree[curr]) queue.push(child);
    }

    return false;
}
