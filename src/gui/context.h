//
// Created by samuel on 06/02/25.
//

#ifndef EVO_MOTION_CONTEXT_H
#define EVO_MOTION_CONTEXT_H

#include <memory>
#include <optional>

#include <evo_motion_model/robot/builder.h>

class AppContext {
public:
    explicit AppContext();

    /*
     * Builder context function
     */

    bool is_member_focused();
    std::string get_focused_member();

    bool is_constraint_focused();
    std::string get_focused_constraint();

    bool is_builder_env_selected();
    std::shared_ptr<RobotBuilderEnvironment> get_builder_env();

    void set_focus_member(const std::string &new_focus_member);
    void release_focus_member();

    void set_focus_constraint(const std::string &new_focus_constraint);
    void release_focus_constraint();

    void set_builder_env(const std::shared_ptr<RobotBuilderEnvironment> &new_env);
    void release_builder_env();

    void hide_members(bool hidden);
    bool are_members_hidden();

    void hide_constraints(bool hidden);
    bool are_constraints_hidden();

    /*
     * Infer helper function
     */
    bool is_robot_infer_json_path_selected();
    std::filesystem::path get_robot_infer_json_path();
    void set_robot_infer_json_path(const std::filesystem::path &robot_json_path);
    void release_robot_infer_json_path();

    bool is_agent_infer_path_selected();
    std::filesystem::path get_agent_infer_path();
    void set_agent_infer_path(const std::filesystem::path &agent_folder_path);
    void release_agent_infer_path();

private:
    std::optional<std::shared_ptr<RobotBuilderEnvironment>> curr_robot_builder_env;
    std::optional<std::string> member_focus;
    std::optional<std::string> constraint_focus;

    std::optional<std::filesystem::path> robot_infer_json_path;
    std::optional<std::filesystem::path> agent_infer_folder_path;

    bool members_hidden;
    bool constraints_hidden;
};

#endif//EVO_MOTION_CONTEXT_H
