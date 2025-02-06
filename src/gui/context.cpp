//
// Created by samuel on 06/02/25.
//

#include "./context.h"

AppContext::AppContext()
    : curr_robot_builder_env(std::nullopt), member_focus(std::nullopt),
      robot_infer_json_path(std::nullopt) {}

/*
 * Robot builder
 */

std::string AppContext::get_focused_member() { return member_focus.value(); }

std::shared_ptr<RobotBuilderEnvironment> AppContext::get_builder_env() {
    return curr_robot_builder_env.value();
}

bool AppContext::is_member_focused() { return member_focus.has_value(); }

bool AppContext::is_builder_env_selected() { return curr_robot_builder_env.has_value(); }

void AppContext::set_focus_member(const std::string &new_focus_member) {
    member_focus = new_focus_member;
}

void AppContext::release_focus_member() { member_focus = std::nullopt; }

void AppContext::set_builder_env(const std::shared_ptr<RobotBuilderEnvironment> &new_env) {
    curr_robot_builder_env = new_env;
}

void AppContext::release_builder_env() { curr_robot_builder_env = std::nullopt; }

/*
 * Robot infer JSON
 */

bool AppContext::is_robot_infer_json_path_selected() { return robot_infer_json_path.has_value(); }

std::filesystem::path AppContext::get_robot_infer_json_path() {
    return robot_infer_json_path.value();
}

void AppContext::set_robot_infer_json_path(const std::filesystem::path &robot_json_path) {
    robot_infer_json_path = robot_json_path;
}

void AppContext::release_robot_infer_json_path() { robot_infer_json_path = std::nullopt; }

/*
 * Agent infer folder
 */

bool AppContext::is_agent_infer_path_selected() { return agent_infer_folder_path.has_value(); }

std::filesystem::path AppContext::get_agent_infer_path() { return agent_infer_folder_path.value(); }

void AppContext::set_agent_infer_path(const std::filesystem::path &agent_folder_path) {
    agent_infer_folder_path = agent_folder_path;
}

void AppContext::release_agent_infer_path() { agent_infer_folder_path = std::nullopt; }
