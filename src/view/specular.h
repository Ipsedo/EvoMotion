//
// Created by samuel on 17/12/22.
//

#ifndef EVO_MOTION_SPECULAR_H
#define EVO_MOTION_SPECULAR_H

#include <vector>
#include <glm/glm.hpp>

#include "constants.h"
#include "program.h"
#include "drawable.h"

class OBjSpecular : public Drawable {
private:
    static const int POSITION_SIZE = 3;
    static const int NORMAL_SIZE = 3;
    static const int STRIDE = (POSITION_SIZE + NORMAL_SIZE) * BYTES_PER_FLOAT;

    Program program;

    glm::vec4 ambient_color;
    glm::vec4 diffuse_color;
    glm::vec4 specular_color;
    float shininess;

    int nb_vertices;


public:
    OBjSpecular(const std::vector<std::tuple<float, float, float>> &vertices,
                const std::vector<std::tuple<float, float, float>> &normals,
                glm::vec4 ambient_color,
                glm::vec4 diffuse_color,
                glm::vec4 specular_color, float shininess);

    void
    draw(glm::mat4 mvp_matrix, glm::mat4 mv_matrix,
         glm::vec3 light_pos_from_camera, glm::vec3 camera_pos) override;

    ~OBjSpecular();
};

#endif //EVO_MOTION_SPECULAR_H
