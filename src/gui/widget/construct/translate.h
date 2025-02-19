//
// Created by samuel on 17/02/25.
//

#ifndef EVO_MOTION_TRANSLATE_H
#define EVO_MOTION_TRANSLATE_H

#include <memory>
#include <optional>
#include <string>

#include <evo_motion_model/robot/builder.h>

class TranslateTools {
public:
    TranslateTools();

    std::optional<glm::vec3> get_pos_delta(
        const bool parent_cond, const std::tuple<float, float, float> &yaw_pitch_roll,
        const glm::mat4 &projection_matrix, const glm::mat4 &view_matrix);

private:
    bool is_dragging;
    float prev_x_mouse;
    float prev_y_mouse;
};

#endif//EVO_MOTION_TRANSLATE_H
