//
// Created by samuel on 13/02/25.
//

#ifndef EVO_MOTION_CONSTRAINT_POPUP_H
#define EVO_MOTION_CONSTRAINT_POPUP_H

#include "../popup.h"

class ConstraintMenuWindow : public PopUpWindow {
public:
    ConstraintMenuWindow(
        const std::string &constraint_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);
    std::optional<std::shared_ptr<ImGuiWindow>> pop_child() override;

protected:
    void render_window_content(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

private:
    std::string constraint_name;
    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    std::optional<std::shared_ptr<ImGuiWindow>> children;

    std::string parent_item;
    std::string child_item;

    void add_focus(const std::shared_ptr<ItemFocusContext> &context);
    void clear_focus(const std::shared_ptr<ItemFocusContext> &context);
};

#endif//EVO_MOTION_CONSTRAINT_POPUP_H
