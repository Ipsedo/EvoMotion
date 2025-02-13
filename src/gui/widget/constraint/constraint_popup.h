//
// Created by samuel on 13/02/25.
//

#ifndef EVO_MOTION_CONSTRAINT_POPUP_H
#define EVO_MOTION_CONSTRAINT_POPUP_H

#include "../popup.h"

class FocusConstraintPopUpWindow : public PopUpWindow {
public:
    FocusConstraintPopUpWindow(
        const std::string &constraint_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);
    std::optional<std::shared_ptr<ImGuiWindow>> pop_child() override;

protected:
    void render_popup_content(const std::shared_ptr<ItemFocusContext> &context) override;
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

/*
 * No focus
 */

class NoFocusConstraintPopUpWindow : public PopUpWindow {
public:
    NoFocusConstraintPopUpWindow(const std::shared_ptr<RobotBuilderEnvironment> &builder_env);
    std::optional<std::shared_ptr<ImGuiWindow>> pop_child() override;

protected:
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;
    void render_popup_content(const std::shared_ptr<ItemFocusContext> &context) override;

private:
    std::shared_ptr<RobotBuilderEnvironment> builder_env;
    std::optional<std::shared_ptr<ImGuiWindow>> children;
};

#endif//EVO_MOTION_CONSTRAINT_POPUP_H
