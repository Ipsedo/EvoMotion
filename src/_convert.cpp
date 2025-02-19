//
// Created by samuel on 24/01/25.
//

#include "./_convert.h"

glm::mat4 to_mat4(nlohmann::json json_transformation) {
    const auto pos = glm::vec3(
        json_transformation["translation"]["x"].get<float>(),
        json_transformation["translation"]["y"].get<float>(),
        json_transformation["translation"]["z"].get<float>());

    const auto rotation = json_transformation["rotation"];
    const auto rot_point = glm::vec3(
        rotation["point"]["x"].get<float>(), rotation["point"]["y"].get<float>(),
        rotation["point"]["z"].get<float>());
    const auto rot_axis = glm::vec3(
        rotation["axis"]["x"].get<float>(), rotation["axis"]["y"].get<float>(),
        rotation["axis"]["z"].get<float>());
    const auto angle_radian = float(M_PI) * rotation["angle_degree"].get<float>() / 180.f;

    const auto translation_to_origin = glm::translate(glm::mat4(1.f), -rot_point);
    const auto rotation_matrix = glm::rotate(glm::mat4(1.f), angle_radian, rot_axis);
    const auto translation_back = glm::translate(glm::mat4(1.f), rot_point);
    const auto translation_to_position = glm::translate(glm::mat4(1.f), pos);

    return translation_to_position * translation_back * rotation_matrix * translation_to_origin;
}

std::tuple<glm::vec3, glm::quat, glm::vec3> decompose(glm::mat4 mat4) {
    auto new_translate = glm::vec3(0.0);
    auto new_rotate = glm::quat();
    auto new_scale = glm::vec3(1.0);
    auto new_skew = glm::vec3(0.0);
    auto new_perspective = glm::vec4(0.0);
    glm::decompose(mat4, new_scale, new_rotate, new_translate, new_skew, new_perspective);
    return {new_translate, new_rotate, new_scale};
}

float round_float(float f, int precision) {
    return std::round(f * std::pow(2.f, static_cast<float>(precision)))
           / std::pow(2.f, static_cast<float>(precision));
}

std::string float_to_binary_string(float f) {
    union {
        float v_f;
        uint32_t bits;
    } data{};
    data.v_f = round_float(f, 5);

    return std::bitset<32>(data.bits).to_string();
}

float binary_string_to_float(const std::string &s) {
    union {
        uint32_t bits;
        float output;
    } data{};

    data.bits = std::bitset<32>(s).to_ulong();

    return data.output;
}

void convert_tree_skeleton_to_graph_skeleton() {

    std::ifstream stream("/home/samuel/CLionProjects/EvoMotion/evo_motion_model/resources/skeleton/"
                         "_old/spider_new.json");

    nlohmann::json robot_data = nlohmann::json::parse(stream);

    std::queue<std::tuple<
        nlohmann::json, glm::mat4, std::vector<std::string>, std::optional<nlohmann::json>>>
        queue;
    queue.emplace(robot_data["skeleton"], glm::mat4(1.0), std::vector<std::string>(), std::nullopt);

    std::vector<nlohmann::json> new_members;
    std::vector<nlohmann::json> new_constraints;
    std::vector<nlohmann::json> new_muscles;

    while (!queue.empty()) {
        const auto [curr_member, parent_model_mat, parent_names, curr_constraint_opt] =
            queue.front();
        queue.pop();

        const auto curr_model_mat = parent_model_mat * to_mat4(curr_member["transformation"]);

        const auto [curr_translate, curr_rotate, curr_scale] = decompose(curr_model_mat);

        std::vector<std::string> new_names(parent_names.begin(), parent_names.end());
        new_names.push_back(curr_member["name"]);

        float friction = 0.5;
        bool ignore_col = false;
        if (curr_member.contains("option")) {
            if (curr_member["option"].contains("friction"))
                friction = curr_member["option"]["friction"].get<float>();

            if (curr_member["option"].contains("ignore_collision"))
                ignore_col = curr_member["option"]["ignore_collision"].get<bool>();
        }

        nlohmann::json new_member = {
            {"name", std::accumulate(
                         std::next(new_names.begin()), new_names.end(), new_names[0],
                         [](const std::string &a, const std::string &b) { return a + "_" + b; })},
            {"mass", float_to_binary_string(curr_member["mass"].get<float>())},
            {"shape", curr_member["shape"].get<std::string>()},
            {"scale",
             {{"x", float_to_binary_string(curr_member["scale"]["x"].get<float>())},
              {"y", float_to_binary_string(curr_member["scale"]["y"].get<float>())},
              {"z", float_to_binary_string(curr_member["scale"]["z"].get<float>())}}},
            {"rotation",
             {{"x", float_to_binary_string(curr_rotate.x)},
              {"y", float_to_binary_string(curr_rotate.y)},
              {"z", float_to_binary_string(curr_rotate.z)},
              {"w", float_to_binary_string(curr_rotate.w)}}},
            {"translation",
             {{"x", float_to_binary_string(curr_translate.x)},
              {"y", float_to_binary_string(curr_translate.y)},
              {"z", float_to_binary_string(curr_translate.z)}}},
            {"friction", float_to_binary_string(friction)},
            {"ignore_collision", ignore_col},
        };

        new_members.push_back(new_member);

        if (curr_constraint_opt.has_value()) {
            const auto curr_constraint = curr_constraint_opt.value();

            nlohmann::json parent_frame, child_frame;
            if (curr_constraint["constraint_type"].get<std::string>() == "hinge") {
                parent_frame = curr_constraint["frame_in_parent"];
                child_frame = curr_constraint["frame_in_child"];
            } else if (curr_constraint["constraint_type"].get<std::string>() == "fixed") {
                parent_frame = curr_constraint["attach_in_parent"];
                child_frame = curr_constraint["attach_in_child"];
            } else exit(1);

            nlohmann::json new_constraint = {
                {"name", "constraint_" + std::to_string(new_constraints.size())},
                {"parent_name",
                 std::accumulate(
                     std::next(parent_names.begin()), parent_names.end(), parent_names[0],
                     [](std::string a, std::string b) { return a + "_" + b; })},
                {"child_name", std::accumulate(
                                   std::next(new_names.begin()), new_names.end(), new_names[0],
                                   [](std::string a, std::string b) { return a + "_" + b; })},
                {"type", curr_constraint["constraint_type"].get<std::string>()}};

            if (curr_constraint["constraint_type"].get<std::string>() == "hinge") {
                float min_angle_rad = curr_constraint["limit_degree"]["min"].get<float>()
                                      * static_cast<float>(M_PI) / 180.f;
                if (min_angle_rad > static_cast<float>(M_PI))
                    min_angle_rad -= 2. * static_cast<float>(M_PI);

                float max_angle_rad = curr_constraint["limit_degree"]["max"].get<float>()
                                      * static_cast<float>(M_PI) / 180.f;
                if (max_angle_rad > static_cast<float>(M_PI))
                    max_angle_rad -= 2. * static_cast<float>(M_PI);

                new_constraint["limit_radian"] = {
                    {"min", float_to_binary_string(min_angle_rad)},
                    {"max", float_to_binary_string(max_angle_rad)},
                };
                glm::mat4 frame_in_parent = to_mat4(parent_frame);
                glm::mat4 frame_in_child = to_mat4(child_frame);

                auto [pivot_in_parent_tr, pivot_in_parent_rot, pivot_in_parent_scale] =
                    decompose(frame_in_parent);
                auto [pivot_in_child_tr, pivot_in_child_rot, pivot_in_child_scale] =
                    decompose(frame_in_child);

                new_constraint["pivot_in_parent"] = {
                    {"x", float_to_binary_string(pivot_in_parent_tr.x)},
                    {"y", float_to_binary_string(pivot_in_parent_tr.y)},
                    {"z", float_to_binary_string(pivot_in_parent_tr.z)},
                };

                new_constraint["pivot_in_child"] = {
                    {"x", float_to_binary_string(pivot_in_child_tr.x)},
                    {"y", float_to_binary_string(pivot_in_child_tr.y)},
                    {"z", float_to_binary_string(pivot_in_child_tr.z)},
                };

                glm::vec3 axis =
                    glm::vec3(frame_in_parent[2][0], frame_in_parent[2][1], frame_in_parent[2][2]);
                glm::vec3 axis_in_world = parent_model_mat * glm::vec4(axis, 0.f);

                glm::vec3 axis_in_parent =
                    glm::inverse(parent_model_mat) * glm::vec4(glm::normalize(axis_in_world), 0.f);
                glm::vec3 axis_in_child =
                    glm::inverse(curr_model_mat) * glm::vec4(glm::normalize(axis_in_world), 0.f);

                new_constraint["axis_in_parent"] = {
                    {"x", float_to_binary_string(axis_in_parent.x)},
                    {"y", float_to_binary_string(axis_in_parent.y)},
                    {"z", float_to_binary_string(axis_in_parent.z)},
                };

                new_constraint["axis_in_child"] = {
                    {"x", float_to_binary_string(axis_in_child.x)},
                    {"y", float_to_binary_string(axis_in_child.y)},
                    {"z", float_to_binary_string(axis_in_child.z)},
                };

            } else if (curr_constraint["constraint_type"].get<std::string>() == "fixed") {
                const auto [p_c_tr, p_c_rot, p_scale] = decompose(to_mat4(parent_frame));
                const auto [c_c_tr, c_c_rot, c_scale] = decompose(to_mat4(child_frame));
                new_constraint["frame_in_parent"] = {
                    {"translation",
                     {
                         {"x", float_to_binary_string(p_c_tr.x)},
                         {"y", float_to_binary_string(p_c_tr.y)},
                         {"z", float_to_binary_string(p_c_tr.z)},
                     }},
                    {"rotation",
                     {
                         {"x", float_to_binary_string(p_c_rot.x)},
                         {"y", float_to_binary_string(p_c_rot.y)},
                         {"z", float_to_binary_string(p_c_rot.z)},
                         {"w", float_to_binary_string(p_c_rot.w)},
                     }}};
                new_constraint["frame_in_child"] = {
                    {"translation",
                     {
                         {"x", float_to_binary_string(c_c_tr.x)},
                         {"y", float_to_binary_string(c_c_tr.y)},
                         {"z", float_to_binary_string(c_c_tr.z)},
                     }},
                    {"rotation",
                     {
                         {"x", float_to_binary_string(c_c_rot.x)},
                         {"y", float_to_binary_string(c_c_rot.y)},
                         {"z", float_to_binary_string(c_c_rot.z)},
                         {"w", float_to_binary_string(c_c_rot.w)},
                     }}};
            }

            new_constraints.push_back(new_constraint);
        }

        for (const auto constraint: curr_member["children"])
            queue.emplace(constraint["child_member"], curr_model_mat, new_names, constraint);
    }

    for (const auto &m: robot_data["muscles"]) {
        nlohmann::json new_muscle = {
            {"name", m["name"].get<std::string>()},
            {"item_a", m["item_a"].get<std::string>()},
            {"item_b", m["item_b"].get<std::string>()},
            {"attach_mass", float_to_binary_string(m["attach_mass"].get<float>())},
            {"attach_scale",
             {{"x", float_to_binary_string(m["attach_scale"]["x"].get<float>())},
              {"y", float_to_binary_string(m["attach_scale"]["y"].get<float>())},
              {"z", float_to_binary_string(m["attach_scale"]["z"].get<float>())}}},
            {"pos_in_a",
             {{"x", float_to_binary_string(m["pos_in_a"]["x"].get<float>())},
              {"y", float_to_binary_string(m["pos_in_a"]["y"].get<float>())},
              {"z", float_to_binary_string(m["pos_in_a"]["z"].get<float>())}}},
            {"pos_in_b",
             {{"x", float_to_binary_string(m["pos_in_b"]["x"].get<float>())},
              {"y", float_to_binary_string(m["pos_in_b"]["y"].get<float>())},
              {"z", float_to_binary_string(m["pos_in_b"]["z"].get<float>())}}},
            {"force", float_to_binary_string(m["force"].get<float>())},
            {"speed", float_to_binary_string(m["speed"].get<float>())},
        };

        new_muscles.push_back(new_muscle);
    }

    nlohmann::json new_robot = {
        {"robot_name", "spider_new"},
        {"root_name", "body"},
        {"members", new_members},
        {"constraints", new_constraints},
        {"muscles", new_muscles}};

    std::ofstream stream_out("/home/samuel/CLionProjects/EvoMotion/evo_motion_model/resources/"
                             "skeleton/_test/new_format_spider_c++-binary-float.json");
    stream_out << new_robot.dump(4);
    stream_out.close();

    return;
}