//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_MEMBER_CONSTRUCT_TOOLS_H
#define EVO_MOTION_MEMBER_CONSTRUCT_TOOLS_H

#include <evo_motion_model/item.h>

#include "../construct/rotate.h"
#include "../construct/translate.h"
#include "../window.h"

class MemberConstructToolsWindow final : public ImGuiWindow {
public:
    MemberConstructToolsWindow(
        const std::string &member_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

    void render_window(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<OpenGlWindow> &gl_window) override;

protected:
    void render_window_content(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<OpenGlWindow> &gl_window) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;
    bool need_close() override;

    void add_focus(const std::shared_ptr<ItemFocusContext> &context);
    void clear_focus(const std::shared_ptr<ItemFocusContext> &context);

private:
    bool is_editing;
    bool changed;
    int edit_choice;

    float yaw;
    float pitch;
    float roll;

    std::string member_name;
    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    std::shared_ptr<NoShapeItem> construct_item;

    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;

    std::unique_ptr<TranslateTools> translate_tools;
    std::unique_ptr<RotateTools> rotate_tools;
};

#endif//EVO_MOTION_MEMBER_CONSTRUCT_TOOLS_H
