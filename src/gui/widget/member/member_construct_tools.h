//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_MEMBER_CONSTRUCT_TOOLS_H
#define EVO_MOTION_MEMBER_CONSTRUCT_TOOLS_H

#include <evo_motion_model/item.h>

#include "../construct/construct_tools.h"

class MemberConstructToolsWindow final : public ConstructToolsWindow {
public:
    MemberConstructToolsWindow(
        const std::string &member_name,
        const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

protected:
    void on_update_pos(const glm::vec3 &pos_delta) override;
    void on_update_rot(const glm::quat &rot_delta) override;
    void on_update_scale(const glm::vec3 &scale_delta) override;

    std::tuple<glm::vec3, glm::quat, glm::vec3> get_construct_item_model_matrix() override;

    void add_focus(const std::shared_ptr<ItemFocusContext> &context) override;
    void clear_focus(const std::shared_ptr<ItemFocusContext> &context) override;

    bool need_close() override;

private:
    std::string member_name;
    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
};

#endif//EVO_MOTION_MEMBER_CONSTRUCT_TOOLS_H
