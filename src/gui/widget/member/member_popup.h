//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_MEMBER_POPUP_H
#define EVO_MOTION_MEMBER_POPUP_H

#include "../popup.h"

/*
 * On focus member popup
 */

class FocusMemberPopUpWindow final : public PopUpWindow {
public:
    FocusMemberPopUpWindow(
        const std::string &member_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);
    std::optional<std::shared_ptr<ImGuiWindow>> pop_child() override;

protected:
    void render_popup_content(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

private:
    std::string member_name;
    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    std::optional<std::shared_ptr<ImGuiWindow>> children;
};

/*
 * No focus member popup
 */

class NoFocusMemberPopUpWindow final : public PopUpWindow {
public:
    explicit NoFocusMemberPopUpWindow(const std::shared_ptr<RobotBuilderEnvironment> &builder_env);
    std::optional<std::shared_ptr<ImGuiWindow>> pop_child() override;

protected:
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;
    void render_popup_content(const std::shared_ptr<ItemFocusContext> &context) override;

private:
    std::shared_ptr<RobotBuilderEnvironment> builder_env;
    std::optional<std::shared_ptr<ImGuiWindow>> children;
};

#endif//EVO_MOTION_MEMBER_POPUP_H
