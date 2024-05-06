//
// Created by samuel on 15/12/22.
//

#ifndef EVO_MOTION_DRAWABLE_H
#define EVO_MOTION_DRAWABLE_H

#include <memory>
#include <vector>

#include <glm/glm.hpp>

class Drawable {
public:
    virtual void draw(
        glm::mat4 mvp_matrix, glm::mat4 mv_matrix, glm::vec3 light_pos_from_camera,
        glm::vec3 camera_pos) = 0;

    virtual ~Drawable();

    class Builder {
    public:
        static std::shared_ptr<Drawable> build_specular_obj(
            const std::vector<std::tuple<float, float, float>> &vertices,
            const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color,
            glm::vec4 diffuse_color, glm::vec4 specular_color, float shininess);
    };
};

#endif//EVO_MOTION_DRAWABLE_H