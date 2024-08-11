//
// Created by samuel on 17/12/22.
//

#ifndef EVO_MOTION_CAMERA_H
#define EVO_MOTION_CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
    virtual glm::vec3 pos() = 0;

    virtual glm::vec3 look() = 0;

    virtual glm::vec3 up() = 0;

    virtual ~Camera();
};

class StaticCamera final : public Camera {
public:
    StaticCamera(glm::vec3 pos, glm::vec3 look, glm::vec3 up);

    glm::vec3 pos() override;

    glm::vec3 look() override;

    glm::vec3 up() override;

private:
    glm::vec3 pos_vec;
    glm::vec3 look_vec;
    glm::vec3 up_vec;
};

#endif//EVO_MOTION_CAMERA_H