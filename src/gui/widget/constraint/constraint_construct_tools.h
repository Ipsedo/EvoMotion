//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_CONSTRAINT_CONSTRUCT_TOOLS_H
#define EVO_MOTION_CONSTRAINT_CONSTRUCT_TOOLS_H

#include "../construct/construct_tools.h"

class HingeConstructToolsWindow final : public ConstructToolsWindow {
public:
    HingeConstructToolsWindow(
        const std::string &constraint_name,
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
    std::string constraint_name;
    std::shared_ptr<RobotBuilderEnvironment> builder_env;
};

#endif//EVO_MOTION_CONSTRAINT_CONSTRUCT_TOOLS_H
