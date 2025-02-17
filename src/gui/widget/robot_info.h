//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_ROBOT_INFO_H
#define EVO_MOTION_ROBOT_INFO_H

#include "./window.h"

class RobotInfoWindow final : public ImGuiWindow {
public:
    explicit RobotInfoWindow(const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_window_content(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<OpenGlWindow> &gl_window) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

private:
    std::shared_ptr<RobotBuilderEnvironment> builder_env;
};

#endif//EVO_MOTION_ROBOT_INFO_H
