//
// Created by samuel on 19/02/25.
//

#ifndef EVO_MOTION_DUPLICATE_MEMBER_H
#define EVO_MOTION_DUPLICATE_MEMBER_H

#include "../window.h"

class DuplicateGroupWindow : public ImGuiWindow {
public:
    DuplicateGroupWindow(
        const std::string &member_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_window_content(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<OpenGlWindow> &gl_window) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

    bool need_close() override;

private:
    std::string member_name;
    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    glm::vec3 pos;

    glm::vec3 rotation_axis;
    float rotation_angle;

    std::string prefix_name;

    void add_focus(const std::shared_ptr<ItemFocusContext> &context) const;
    void clear_focus(const std::shared_ptr<ItemFocusContext> &context) const;
};

#endif//EVO_MOTION_DUPLICATE_MEMBER_H
