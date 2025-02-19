//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_CONSTRAINT_SETTINGS_H
#define EVO_MOTION_CONSTRAINT_SETTINGS_H

#include "../window.h"

class ConstraintSettingsWindow : public ImGuiWindow {
public:
    ConstraintSettingsWindow(
        const std::string &constraint_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_window_content(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<OpenGlWindow> &gl_window) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;
    bool need_close() override;

    virtual void render_constraint_specific_window(
        const std::string &constraint_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env,
        const std::shared_ptr<ItemFocusContext> &context) = 0;

private:
    std::string constraint_name;
    std::string parent_name;
    std::string child_name;

    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    void add_focus(const std::shared_ptr<ItemFocusContext> &context) const;
    void clear_focus(const std::shared_ptr<ItemFocusContext> &context) const;
};

/*
 * Hinge
 */

class HingeConstraintSettingsWindow : public ConstraintSettingsWindow {
public:
    HingeConstraintSettingsWindow(
        const std::string &constraint_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_constraint_specific_window(
        const std::string &constraint_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env,
        const std::shared_ptr<ItemFocusContext> &context) override;
};

/*
 * Fixed
 */

class FixedConstraintSettingsWindow : public ConstraintSettingsWindow {
public:
    FixedConstraintSettingsWindow(
        const std::string &constraint_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_constraint_specific_window(
        const std::string &constraint_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env,
        const std::shared_ptr<ItemFocusContext> &context) override;
};

#endif//EVO_MOTION_CONSTRAINT_SETTINGS_H
