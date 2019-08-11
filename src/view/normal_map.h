//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_NORMAL_MAP_H
#define EVOMOTION_NORMAL_MAP_H

#include <GL/glew.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>


class normal_map_obj {
private:
    const int POSITION_SIZE = 3;
    const int TEX_COORD_SIZE = 2;
    const int NORMAL_SIZE = 3;
    const int BYTES_PER_FLOAT = 4;
    const int STRIDE = (POSITION_SIZE + NORMAL_SIZE + TEX_COORD_SIZE) * BYTES_PER_FLOAT;

    GLuint m_program;
    GLuint m_position_handle;
    GLuint m_normal_handle;
    GLuint m_text_coord_handle;
    GLuint m_mvp_matrix_handle;
    GLuint m_light_pos_handle;
    GLuint m_cam_pos_handle;
    GLuint m_mv_matrix_handle;
    GLuint m_distance_coef_handle;
    GLuint m_light_coef_handle;
    GLuint m_tex_handle;
    GLuint m_normal_map_handle;

    GLuint buffer;

    GLuint *textures;

    int nb_vertex;

    void init_prgm();

    void init_tex(std::string texture_file, std::string normals_file);

    void bind();

    void gen_buffer(std::string obj_file_name);

    std::vector<float> parse_obj(std::string obj_file_name);

public:
    normal_map_obj(std::string obj_file, std::string texture_file, std::string normals_file);

    void draw(glm::mat4 mvp_matrix, glm::mat4 mv_matrix, glm::vec3 light_pos, glm::vec3 cam_pos);

    ~normal_map_obj();
};

#endif //EVOMOTION_NORMAL_MAP_H
