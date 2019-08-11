//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_OBJ_MTL_VBO_H
#define EVOMOTION_OBJ_MTL_VBO_H

#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>
#include <string>

class ObjMtlVBO {

private:
    /**
     * Constants
     */
    const int POSITION_DATA_SIZE = 3;
    const int NORMAL_DATA_SIZE = 3;
    const int COLOR_DATA_SIZE = 4;
    const int SHININESS_DATA_SIZE = 1;
    const int BYTES_PER_FLOAT = 4;
    const int STRIDE =
            (POSITION_DATA_SIZE
             + NORMAL_DATA_SIZE
             + COLOR_DATA_SIZE * 3
             + SHININESS_DATA_SIZE) * BYTES_PER_FLOAT;

    /**
     * GL program handle
     */
    GLuint m_program;
    GLuint m_position_handle;
    GLuint m_normal_handle;
    GLuint m_amb_color_handle;
    GLuint m_diff_color_handle;
    GLuint m_spec_color_handle;
    GLuint m_spec_shininess_handle;
    GLuint m_camera_pos_handle;
    GLuint m_mvp_matrix_handle;
    GLuint m_light_pos_handle;
    GLuint m_mv_matrix_handle;
    GLuint m_distance_coef_handle;
    GLuint m_light_coef_handle;

    /**
     * Model infos
     */
    float light_coef;
    float distance_coef;
    int nb_vertex;

    /**
     * VBO id
     */
    GLuint packed_data_buffer_id;

    bool random_color;

    void init();

    void bind();

    void bind_buffer(std::vector<float> packed_data);

    std::vector<float> parse_obj(std::string obj_file_name, std::string mtl_file_name);

public:
    ObjMtlVBO(std::string obj_file_name, std::string mtl_file_name, bool will_random_color);

    void draw(glm::mat4 mvp_matrix, glm::mat4 mv_matrix, glm::vec3 ligh_pos_in_eye_space, glm::vec3 camera_pos);
};

#endif //EVOMOTION_OBJ_MTL_VBO_H
