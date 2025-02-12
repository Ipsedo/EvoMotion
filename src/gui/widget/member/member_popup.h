//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_MEMBER_POPUP_H
#define EVO_MOTION_MEMBER_POPUP_H

#include "../window.h"

class MemberMenuWindow : public ImGuiWindow {
public:
    MemberMenuWindow(
        const std::string &member_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);
    std::optional<std::shared_ptr<ImGuiWindow>> pop_child() override;
    void render_window(const std::shared_ptr<ItemFocusContext> &context) override;

protected:
    void render_window_content(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

private:
    bool first_open;

    std::string member_name;
    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    std::optional<std::shared_ptr<ImGuiWindow>> children;
};

#endif//EVO_MOTION_MEMBER_POPUP_H
