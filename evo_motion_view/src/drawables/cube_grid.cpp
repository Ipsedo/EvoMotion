//
// Created by samuel on 19/02/25.
//

#include "./cube_grid.h"

#include <algorithm>

CubeGrid::CubeGrid(const float cube_size, const float cell_size, const glm::vec4 &color)
    : nb_lines(0), program(Program::Builder("./simple_vs.glsl", "./simple_fs.glsl")
                               .add_uniform("u_color")
                               .add_uniform("u_p_matrix")
                               .add_uniform("u_v_matrix")
                               .add_buffer("lines", get_line_data(cube_size, cell_size))
                               .add_attribute("a_position")
                               .build()),
      color(color) {}

std::vector<float> CubeGrid::get_line_data(float cube_size, float cell_size) {
    int num_lines = static_cast<int>(cube_size / cell_size) + 1;

    float side = cube_size / 2.f;

    std::vector<float> packed_data;

    for (int i = 0; i < num_lines; ++i) {
        float offset = i * cell_size;

        packed_data.insert(packed_data.end(), {0, 0, offset, cube_size, 0, offset});
        packed_data.insert(packed_data.end(), {0, cube_size, offset, cube_size, cube_size, offset});
        packed_data.insert(packed_data.end(), {offset, 0, 0, offset, 0, cube_size});
        packed_data.insert(packed_data.end(), {offset, cube_size, 0, offset, cube_size, cube_size});

        packed_data.insert(packed_data.end(), {0, offset, 0, 0, offset, cube_size});
        packed_data.insert(packed_data.end(), {cube_size, offset, 0, cube_size, offset, cube_size});
        packed_data.insert(packed_data.end(), {0, 0, offset, 0, cube_size, offset});
        packed_data.insert(packed_data.end(), {cube_size, 0, offset, cube_size, cube_size, offset});

        packed_data.insert(packed_data.end(), {offset, 0, 0, offset, cube_size, 0});
        packed_data.insert(packed_data.end(), {0, offset, 0, cube_size, offset, 0});
        packed_data.insert(packed_data.end(), {offset, 0, cube_size, offset, cube_size, cube_size});
        packed_data.insert(packed_data.end(), {0, offset, cube_size, cube_size, offset, cube_size});
    }

    std::vector<float> centered_packed_date;
    std::ranges::transform(
        packed_data, std::back_inserter(centered_packed_date),
        [side](const float f) { return f - side; });

    nb_lines = packed_data.size() / 3;

    return centered_packed_date;
}

void CubeGrid::draw(
    glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix,
    glm::vec3 light_pos_from_camera, glm::vec3 camera_pos) {
    glEnable(GL_BLEND);

    program.use();

    program.uniform_mat4("u_p_matrix", projection_matrix);
    program.uniform_mat4("u_v_matrix", view_matrix);
    program.uniform_vec4("u_color", color);

    program.attrib("a_position", "lines", 3, 3 * 4, 0);

    Program::draw_arrays(GL_LINES, 0, nb_lines);

    program.disable_attrib_array();

    glDisable(GL_BLEND);
}
