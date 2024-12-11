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

    virtual void step(float delta) = 0;
    virtual void reset() = 0;

    virtual ~Camera();
};

class StaticCamera final : public Camera {
public:
    StaticCamera(glm::vec3 pos, glm::vec3 look, glm::vec3 up);

    glm::vec3 pos() override;

    glm::vec3 look() override;

    glm::vec3 up() override;

    void step(float delta) override;

    void reset() override;

private:
    glm::vec3 pos_vec;
    glm::vec3 look_vec;
    glm::vec3 up_vec;
};

class FollowCamera final : public Camera {
public:
    explicit FollowCamera(std::function<glm::vec3()> get_object_center);

    glm::vec3 pos() override;

    glm::vec3 look() override;

    glm::vec3 up() override;

    void step(float delta) override;

    void reset() override;

private:
    std::function<glm::vec3()> get_object_center;

    glm::vec3 object_smooth_pos;

    float distance;
    float up_angle;
    float side_angle;

    glm::vec3 pos_vec;
    glm::vec3 look_vec;
    glm::vec3 up_vec;

    float factor;
};

#endif//EVO_MOTION_CAMERA_H