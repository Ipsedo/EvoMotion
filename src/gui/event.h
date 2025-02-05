//
// Created by samuel on 31/01/25.
//

#ifndef EVO_MOTION_EVENT_H
#define EVO_MOTION_EVENT_H

#include <optional>

#include <glm/glm.hpp>

class MouseEvent {
public:
    MouseEvent(float width, float height);

    void update(
        float new_width, float new_height, const glm::mat4 &new_view_matrix,
        const glm::mat4 &new_proj_matrix);

    std::optional<std::tuple<glm::vec3, glm::vec3>>
    get_scene_absolute_click_pos(float width_offset, float height_offset);

private:
    float width;
    float height;

    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
};

#endif//EVO_MOTION_EVENT_H
