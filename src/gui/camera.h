//
// Created by samuel on 29/01/25.
//

#ifndef EVO_MOTION_GUI_CAMERA_H
#define EVO_MOTION_GUI_CAMERA_H

#include <optional>

#include <evo_motion_model/item.h>
#include <evo_motion_view/camera.h>

class ImGuiCamera : public Camera {
public:
    ImGuiCamera();

    void set_focus(const std::function<glm::vec3()> &get_object_center);
    void release_focus();
    void update();

    glm::vec3 pos() override;
    glm::vec3 look() override;
    glm::vec3 up() override;
    void step(float delta) override;
    void reset() override;

private:
    bool in_action;
    int last_x;
    int last_y;

    float factor;

    float vertical_angle;
    float horizontal_angle;
    float distance;

    std::optional<std::function<glm::vec3()>> get_item_pos;
};

#endif//EVO_MOTION_GUI_CAMERA_H
