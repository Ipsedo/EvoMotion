//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_NEW_CONSTRAINT_H
#define EVO_MOTION_NEW_CONSTRAINT_H

#include "../window.h"

class NewConstraintWindow : public ImGuiWindow {
public:
    NewConstraintWindow(
        const ConstraintType &constraint_type,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_window_content(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

    virtual void render_constraint_specific_settings(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env,

        const std::string &constraint_name, const std::optional<std::string> &parent_name,
        const std::optional<std::string> &child_name, const glm::vec3 &absolute_position) = 0;

private:
    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    std::string constraint_name;
    std::optional<std::string> parent_name;
    std::optional<std::string> child_name;

    glm::vec3 absolute_position;

    void add_focus(const std::shared_ptr<ItemFocusContext> &context) const;
    void clear_focus(const std::shared_ptr<ItemFocusContext> &context) const;

    static std::string get_window_name(const ConstraintType &constraint_type);
};

/*
 * Fixed
 */

class NewFixedConstraintWindow final : public NewConstraintWindow {
public:
    explicit NewFixedConstraintWindow(const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_constraint_specific_settings(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env,
        const std::string &constraint_name, const std::optional<std::string> &parent_name,
        const std::optional<std::string> &child_name, const glm::vec3 &absolute_position) override;

private:
    glm::vec3 rotation_axis;
    float angle;
};

/*
 * Hinge
 */

class NewHingeConstraintWindow final : public NewConstraintWindow {
public:
    explicit NewHingeConstraintWindow(const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_constraint_specific_settings(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env,
        const std::string &constraint_name, const std::optional<std::string> &parent_name,
        const std::optional<std::string> &child_name, const glm::vec3 &absolute_position) override;

private:
    glm::vec3 hinge_axis;
};

#endif//EVO_MOTION_NEW_CONSTRAINT_H
