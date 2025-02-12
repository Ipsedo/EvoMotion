//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_CONSTRAINT_SETTINGS_H
#define EVO_MOTION_CONSTRAINT_SETTINGS_H

#include "../window.h"

class ConstraintSettingsWindow final : public ImGuiWindow {
public:
    ConstraintSettingsWindow(
        const std::string &constraint_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_window_content(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

private:
    std::string constraint_name;
    std::string parent_name;
    std::string child_name;

    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    void add_focus(const std::shared_ptr<ItemFocusContext> &context);
    void clear_focus(const std::shared_ptr<ItemFocusContext> &context);
};

#endif//EVO_MOTION_CONSTRAINT_SETTINGS_H
