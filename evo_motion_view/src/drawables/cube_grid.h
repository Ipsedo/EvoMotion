//
// Created by samuel on 19/02/25.
//

#ifndef EVO_MOTION_CUBE_GRID_H
#define EVO_MOTION_CUBE_GRID_H

#include <evo_motion_view/drawable.h>

#include "../program.h"

class CubeGrid : public Drawable {
public:
    CubeGrid(float cube_size, float cell_size, const glm::vec4 &color);

    void draw(
        glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix,
        glm::vec3 light_pos_from_camera, glm::vec3 camera_pos) override;

private:
    std::vector<float> get_line_data(float cube_size, float cell_size);

    int nb_lines;
    Program program;
    glm::vec4 color;
};

#endif//EVO_MOTION_CUBE_GRID_H
