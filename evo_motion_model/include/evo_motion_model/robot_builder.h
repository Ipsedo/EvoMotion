//
// Created by samuel on 15/01/25.
//

#ifndef EVO_MOTION_ROBOT_BUILDER_H
#define EVO_MOTION_ROBOT_BUILDER_H

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "./environment.h"
#include "./skeleton.h"

/*
 * Parameter Member
 */

class BuilderConstraint;

class BuilderMember : public std::enable_shared_from_this<BuilderMember>, public AbstractMember {
public:
    BuilderMember(
        const std::string &name, ShapeKind shape_kind, glm::vec3 center_pos, glm::quat rotation,
        glm::vec3 scale, float mass, float friction);

    std::string get_name();
    float get_mass();
    glm::vec3 get_scale();
    float get_friction();
    ShapeKind get_shape();

    void transform(
        std::optional<glm::vec3> new_center_pos, std::optional<glm::quat> new_rotation,
        std::optional<glm::vec3> new_scale);

    Item get_item() override;
    std::vector<std::shared_ptr<AbstractConstraint>> get_children() override;
    std::vector<std::shared_ptr<BuilderConstraint>> get_builder_children();

    void add_fixed_constraint(
        const Item &parent, const glm::mat4 &attach_in_parent, const glm::mat4 &attach_in_child);
    void add_hinge_constraint(
        const Item &parent, const glm::mat4 &frame_in_parent, const glm::mat4 &frame_in_child,
        float limit_degree_min, float limit_degree_max);

private:
    std::map<ShapeKind, std::string> shape_to_path;
    ShapeKind shape;
    std::string name;
    Item member;
    std::vector<std::shared_ptr<BuilderConstraint>> constraints;

    void propagate_transform(glm::mat4 new_model_matrix);

    template<typename ConstraintConstructor, typename... Args>
    void add_constraint(Args... args);
};

/*
 * Constraint
 */

class BuilderConstraint : public AbstractConstraint {
public:
    explicit BuilderConstraint(const std::shared_ptr<BuilderMember> &child);
    std::shared_ptr<BuilderMember> get_builder_member_child();

protected:
    std::shared_ptr<BuilderMember> child;
};

class BuilderFixedConstraint : public BuilderConstraint {
public:
    BuilderFixedConstraint(
        const Item &parent, const glm::mat4 &attach_in_parent, const glm::mat4 &attach_in_child,
        const std::shared_ptr<BuilderMember> &child);
    btTypedConstraint *get_constraint() override;
    std::shared_ptr<AbstractMember> get_child() override;

private:
    btFixedConstraint *constraint;
};

class BuilderHingeConstraint : public BuilderConstraint {
public:
    BuilderHingeConstraint(
        const Item &parent, const glm::mat4 frame_in_parent, const glm::mat4 frame_in_child,
        float limit_degree_min, float limit_degree_max,
        const std::shared_ptr<BuilderMember> &child);

    btTypedConstraint *get_constraint() override;
    std::shared_ptr<AbstractMember> get_child() override;

private:
    btHingeConstraint *constraint;
};

/*
 * Environment
 */

class RobotBuilderEnvironment : public Environment {
public:
    RobotBuilderEnvironment();

    bool add_root(
        const std::string &name, glm::vec3 center_pos, glm::vec3 scale, float mass, float friction);
    bool add_item(
        const std::string &name, glm::vec3 center_pos, glm::vec3 scale, float mass, float friction);
    bool attach_item_fixed_constraint(const std::string &name, const std::string &parent);
    bool attach_item_hinge_constraint(const std::string &name, const std::string &parent);
    bool attach_item_cone_constraint(const std::string &name, const std::string &parent);

    void robot_to_json(const std::filesystem::path &output_path, const std::string &robot_name);
    void json_to_robot(const std::filesystem::path &json_path, const std::string &robot_name);

private:
    std::map<std::string, std::vector<std::string>> skeleton_tree;
    std::shared_ptr<BuilderMember> root;
    std::vector<std::shared_ptr<BuilderMember>> members_not_attached;

    std::map<ShapeKind, std::string> shape_to_name;

    bool will_produce_cycle(const std::string &parent, const std::string &member);
    bool member_exists(const std::string &name);
};

#endif//EVO_MOTION_ROBOT_BUILDER_H
