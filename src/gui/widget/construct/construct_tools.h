//
// Created by samuel on 21/02/25.
//

#ifndef EVO_MOTION_CONSTRUCT_TOOLS_H
#define EVO_MOTION_CONSTRUCT_TOOLS_H

#include "../window.h"
#include "./rotate.h"
#include "./translate.h"

class ConstructToolsWindow : public ImGuiWindow {
public:
    ConstructToolsWindow(const std::string &window_name, bool position, bool rotation, bool scale);

    void render_window(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<OpenGlWindow> &gl_window) override;

protected:
    void render_window_content(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<OpenGlWindow> &gl_window) override;

    virtual void on_update_pos(const glm::vec3 &pos_delta) = 0;
    virtual void on_update_rot(const glm::quat &rot_delta) = 0;
    virtual void on_update_scale(const glm::vec3 &scale_delta) = 0;

    virtual std::tuple<glm::vec3, glm::quat, glm::vec3> get_construct_item_model_matrix() = 0;

    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

    virtual void add_focus(const std::shared_ptr<ItemFocusContext> &context) = 0;
    virtual void clear_focus(const std::shared_ptr<ItemFocusContext> &context) = 0;

private:
    bool is_editing;
    bool changed;

    bool position_tools;
    bool rotation_tools;
    bool scale_tools;
    int edit_choice;

    float yaw;
    float pitch;
    float roll;

    std::shared_ptr<NoShapeItem> construct_item;

    std::unique_ptr<TranslateTools> translate_tools;
    std::unique_ptr<RotateTools> rotate_tools;
};

#endif//EVO_MOTION_CONSTRUCT_TOOLS_H
