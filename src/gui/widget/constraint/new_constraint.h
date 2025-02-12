//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_NEW_CONSTRAINT_H
#define EVO_MOTION_NEW_CONSTRAINT_H

#include "../window.h"

class NewConstraintWindow final : public ImGuiWindow {
public:
    NewConstraintWindow(const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_window_content(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

private:
    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    std::string constraint_name;
    std::optional<std::string> parent_name;
    std::optional<std::string> child_name;

    glm::vec3 absolute_position;

    void add_focus(const std::shared_ptr<ItemFocusContext> &context);
    void clear_focus(const std::shared_ptr<ItemFocusContext> &context);
};

#endif//EVO_MOTION_NEW_CONSTRAINT_H
