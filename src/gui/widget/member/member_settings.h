//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_MEMBER_SETTINGS_H
#define EVO_MOTION_MEMBER_SETTINGS_H

#include "../window.h"

class MemberSettingsWindow final : public ImGuiWindow {
public:
    MemberSettingsWindow(
        const std::string &member_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_window_content(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;

    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

private:
    std::string member_name;
    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    void add_focus(const std::shared_ptr<ItemFocusContext> &context);
    void clear_focus(const std::shared_ptr<ItemFocusContext> &context);
};

#endif//EVO_MOTION_MEMBER_SETTINGS_H
