//
// Created by samuel on 15/12/22.
//

#ifndef EVO_MOTION_DRAWABLE_H
#define EVO_MOTION_DRAWABLE_H

#include <memory>
#include <random>
#include <vector>

#include <glm/glm.hpp>

class Drawable {
public:
    virtual void draw(
        glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix,
        glm::vec3 light_pos_from_camera, glm::vec3 camera_pos) = 0;

    virtual ~Drawable() = default;
};

class DrawableFactory {
public:
    virtual ~DrawableFactory() = default;

    virtual std::shared_ptr<Drawable> create_drawable() = 0;
};

#endif//EVO_MOTION_DRAWABLE_H