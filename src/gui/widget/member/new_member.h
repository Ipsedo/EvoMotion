//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_NEW_MEMBER_H
#define EVO_MOTION_NEW_MEMBER_H

#include "../window.h"

class NewMemberWindow final : public ImGuiWindow {
public:
    explicit NewMemberWindow(const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void render_window_content(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

private:
    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    std::string member_name;

    glm::vec3 pos;

    glm::vec3 rotation_axis;
    float rotation_angle;

    glm::vec3 scale;

    float mass;
    float friction;
};

#endif//EVO_MOTION_NEW_MEMBER_H
