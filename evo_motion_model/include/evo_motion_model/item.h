//
// Created by samuel on 18/12/22.
//

#ifndef EVO_MOTION_ITEM_H
#define EVO_MOTION_ITEM_H

#include <functional>
#include <memory>
#include <string>

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "./shapes.h"

enum DrawableKind { SPECULAR, TILE_SPECULAR };

/*
 * Abstract
 */

class EmptyItem {
public:
    [[nodiscard]] virtual std::string get_name() const = 0;

    [[nodiscard]] virtual glm::mat4 model_matrix() const = 0;
    [[nodiscard]] virtual glm::mat4 model_matrix_without_scale() const = 0;

    virtual void reset(const glm::mat4 &main_model_matrix) = 0;

    virtual ~EmptyItem() = default;
};

class ShapeItem : public EmptyItem {
public:
    [[nodiscard]] virtual std::shared_ptr<Shape> get_shape() const = 0;

    [[nodiscard]] virtual DrawableKind get_drawable_kind() const = 0;
};

/*
 * Rigid body
 */

class RigidBodyItem final : public ShapeItem {
public:
    RigidBodyItem(
        std::string name, const std::shared_ptr<Shape> &shape, const glm::mat4 &model_matrix,
        const glm::vec3 &scale, float mass, const DrawableKind &drawable_kind);

    RigidBodyItem(
        std::string name, const std::shared_ptr<Shape> &shape, const glm::vec3 &position,
        const glm::vec3 &scale, float mass, const DrawableKind &drawable_kind);

    RigidBodyItem(
        std::string name, const std::shared_ptr<Shape> &shape, const glm::vec3 &position,
        const glm::quat &rotation, const glm::vec3 &scale, float mass,
        const DrawableKind &drawable_kind);

    [[nodiscard]] std::shared_ptr<Shape> get_shape() const override;
    [[nodiscard]] std::string get_name() const override;

    [[nodiscard]] btRigidBody *get_body() const;

    [[nodiscard]] glm::mat4 model_matrix() const override;
    [[nodiscard]] glm::mat4 model_matrix_without_scale() const override;

    void rename(const std::string &new_name);

    void reset(const glm::mat4 &main_model_matrix) override;

    [[nodiscard]] DrawableKind get_drawable_kind() const override;

private:
    std::string name;

    std::shared_ptr<Shape> shape;

    btRigidBody *body;
    btCollisionShape *collision_shape;

    glm::mat4 first_model_matrix;

    DrawableKind kind;
};

/*
 * No Shape Item
 */

enum PredefinedDrawableKind { BASIS_AXIS, ROTATION_TORUS };

class NoShapeItem final : public EmptyItem {
public:
    NoShapeItem(std::string name, const PredefinedDrawableKind &predefined_drawable);

    [[nodiscard]] std::string get_name() const override;
    [[nodiscard]] glm::mat4 model_matrix() const override;
    [[nodiscard]] glm::mat4 model_matrix_without_scale() const override;
    void reset(const glm::mat4 &main_model_matrix) override;

    [[nodiscard]] PredefinedDrawableKind get_drawable_kind() const;
    void set_drawable_kind(const PredefinedDrawableKind &new_drawable_kind);

private:
    std::string name;

    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

    PredefinedDrawableKind predefined_drawable;
};

/*
 * NoBodyItem
 */

class NoBodyItem final : public ShapeItem {
public:
    NoBodyItem(
        const std::string &name, const std::shared_ptr<Shape> &shape, const glm::vec3 &position,
        const glm::quat &rotation, const glm::vec3 &scale, const DrawableKind &drawable_kind);

    NoBodyItem(
        std::string name, const std::shared_ptr<Shape> &shape,
        const std::function<glm::vec3()> &get_position,
        const std::function<glm::quat()> &get_rotation, const std::function<glm::vec3()> &get_scale,
        const DrawableKind &drawable_kind);

    [[nodiscard]] std::string get_name() const override;
    [[nodiscard]] glm::mat4 model_matrix() const override;
    [[nodiscard]] glm::mat4 model_matrix_without_scale() const override;
    void reset(const glm::mat4 &main_model_matrix) override;

    [[nodiscard]] std::shared_ptr<Shape> get_shape() const override;
    [[nodiscard]] DrawableKind get_drawable_kind() const override;

private:
    std::string name;

    std::function<glm::vec3()> get_position;
    std::function<glm::quat()> get_rotation;
    std::function<glm::vec3()> get_scale;

    glm::mat4 first_model_matrix;

    std::shared_ptr<Shape> shape;
    DrawableKind drawable_kind;
};

#endif//EVO_MOTION_ITEM_H