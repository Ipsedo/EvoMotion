//
// Created by samuel on 15/01/25.
//

#ifndef EVO_MOTION_BUILDER_H
#define EVO_MOTION_BUILDER_H

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "../environment.h"
#include "./constraint.h"
#include "./member.h"
#include "./muscle.h"
#include "./skeleton.h"

/*
 * Parameter Member
 */

class BuilderMember : public Member, public std::enable_shared_from_this<BuilderMember> {
public:
    BuilderMember(
        const std::string &name, ShapeKind shape_kind, const glm::vec3 &center_pos,
        const glm::quat &rotation, const glm::vec3 &scale, float mass, float friction,
        bool ignore_collision);

    explicit BuilderMember(const std::shared_ptr<AbstractDeserializer> &deserializer);

    void update_item(
        const std::optional<glm::vec3> &new_pos = std::nullopt,
        const std::optional<glm::quat> &new_rot = std::nullopt,
        const std::optional<glm::vec3> &new_scale = std::nullopt,
        const std::optional<float> &new_friction = std::nullopt,
        const std::optional<float> &new_mass = std::nullopt,
        const std::optional<bool> &new_ignore_collision = std::nullopt);
};

/*
 * Builder Constraints
 */

enum ConstraintType { FIXED, HINGE };

class BuilderConstraint : public virtual Constraint {
public:
    BuilderConstraint(
        const std::string &name, const std::shared_ptr<Member> &parent,
        const std::shared_ptr<Member> &child);
    BuilderConstraint(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)> &get_member_function);

    btRigidBody *create_fake_body();

    virtual std::vector<std::shared_ptr<NoBodyItem>> get_builder_empty_items() = 0;

protected:
    virtual std::shared_ptr<Shape> get_shape() = 0;
};

class BuilderHingeConstraint final : public virtual HingeConstraint,
                                     public virtual BuilderConstraint {
public:
    BuilderHingeConstraint(
        const std::string &name, const std::shared_ptr<Member> &parent,
        const std::shared_ptr<Member> &child, const glm::vec3 &pivot_in_parent,
        const glm::vec3 &pivot_in_child, const glm::vec3 &axis_in_parent,
        const glm::vec3 &axis_in_child, float limit_radian_min, float limit_radian_max);

    BuilderHingeConstraint(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)> &get_member_function);

    void update_constraint(
        const std::optional<glm::vec3> &new_pivot = std::nullopt,
        const std::optional<glm::vec3> &new_axis = std::nullopt,
        const std::optional<float> &new_limit_radian_min = std::nullopt,
        const std::optional<float> &new_limit_radian_max = std::nullopt);

    std::vector<std::shared_ptr<NoBodyItem>> get_builder_empty_items() override;

protected:
    std::shared_ptr<Shape> get_shape() override;

private:
    std::shared_ptr<Shape> shape;
    std::shared_ptr<Shape> angle_limit_start_shape;
    std::shared_ptr<Shape> angle_limit_end_shape;
    std::shared_ptr<Shape> angle_ref_shape;
};

class BuilderFixedConstraint final : public virtual FixedConstraint,
                                     public virtual BuilderConstraint {
public:
    BuilderFixedConstraint(
        const std::string &name, const std::shared_ptr<Member> &parent,
        const std::shared_ptr<Member> &child, const glm::mat4 &frame_in_parent,
        const glm::mat4 &frame_in_child);

    BuilderFixedConstraint(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)> &get_member_function);

    void update_constraint(
        const std::optional<glm::vec3> &new_pivot = std::nullopt,
        const std::optional<glm::quat> &new_rot = std::nullopt);

    std::vector<std::shared_ptr<NoBodyItem>> get_builder_empty_items() override;

protected:
    std::shared_ptr<Shape> get_shape() override;

private:
    std::shared_ptr<Shape> shape;
};

/*
 * Muscles
 */

class BuilderMuscle final : public Muscle {
public:
    BuilderMuscle(
        const std::string &name, float attach_mass, const glm::vec3 &attach_scale,
        const std::shared_ptr<RigidBodyItem> &item_a, const glm::vec3 &pos_in_a,
        const std::shared_ptr<RigidBodyItem> &item_b, const glm::vec3 &pos_in_b, float force,
        float max_speed);

    BuilderMuscle(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)> &get_member_function);
};

/*
 * Environment
 */

class RobotBuilderEnvironment final : public Environment {
public:
    explicit RobotBuilderEnvironment(std::string robot_name);

    bool set_root(const std::string &member_name);

    bool add_member(
        const std::string &member_name, const ShapeKind &shape_kind, const glm::vec3 &center_pos,
        const glm::quat &rotation, const glm::vec3 &scale, float mass, float friction);

    bool clone_body_part(
        const std::string &member_name, const std::string &prefix_name, const glm::vec3 &center_pos,
        const glm::quat &rotation);

    bool update_member(
        const std::string &member_name, const std::optional<glm::vec3> &new_pos = std::nullopt,
        const std::optional<glm::quat> &new_rot = std::nullopt,
        const std::optional<glm::vec3> &new_scale = std::nullopt,
        const std::optional<float> &new_friction = std::nullopt,
        const std::optional<float> &new_mass = std::nullopt,
        const std::optional<bool> &new_ignore_collision = std::nullopt);

    bool update_hinge_constraint(
        const std::string &hinge_constraint_name,
        const std::optional<glm::vec3> &new_pos = std::nullopt,
        const std::optional<glm::vec3> &new_axis = std::nullopt,
        const std::optional<float> &new_limit_angle_min = std::nullopt,
        const std::optional<float> &new_angle_limit_max = std::nullopt) const;

    bool update_fixed_constraint(
        const std::string &fixed_constraint_name,
        const std::optional<glm::vec3> &new_pos = std::nullopt,
        const std::optional<glm::quat> &new_rot = std::nullopt) const;

    bool rename_member(const std::string &old_name, const std::string &new_name);

    bool attach_fixed_constraint(
        const std::string &constraint_name, const std::string &parent_name,
        const std::string &child_name, const glm::vec3 &absolute_fixed_point,
        const glm::quat &absolute_rotation);

    bool remove_member(const std::string &member_name);
    bool remove_constraint(const std::string &constraint_name);

    std::tuple<glm::vec3, glm::quat, glm::vec3>
    get_member_transform(const std::string &member_name) const;

    float get_member_mass(const std::string &member_name) const;
    float get_member_friction(const std::string &member_name) const;

    ConstraintType get_constraint_type(const std::string &constraint_name) const;
    std::tuple<glm::vec3, glm::vec3, float, float>
    get_constraint_hinge_info(const std::string &hinge_constraint_name) const;
    std::tuple<glm::vec3, glm::quat>
    get_constraint_fixed_info(const std::string &fixed_constraint_name) const;
    std::tuple<std::string, std::string> get_constraint_members(const std::string &constraint_name);

    std::optional<std::string>
    ray_cast_member(const glm::vec3 &from_absolute, const glm::vec3 &to_absolute) const;

    std::optional<std::string>
    ray_cast_constraint(const glm::vec3 &from_absolute, const glm::vec3 &to_absolute);

    void save_robot(const std::filesystem::path &output_json_path);
    void load_robot(const std::filesystem::path &input_json_path);

    std::string get_robot_name();
    std::optional<std::string> get_root_name();
    void set_robot_name(const std::string &new_robot_name);

    std::vector<std::string> get_member_names();

    int get_members_count() const;

    bool member_exists(const std::string &member_name) const;
    bool constraint_exists(const std::string &constraint_name) const;
    bool muscle_exists(const std::string &muscle_name) const;

    /*
     * Environment
     */

    std::vector<std::shared_ptr<ShapeItem>> get_draw_items() override;
    std::vector<std::shared_ptr<Controller>> get_controllers() override;
    std::vector<int64_t> get_state_space() override;
    std::vector<int64_t> get_action_space() override;
    std::optional<std::shared_ptr<ShapeItem>> get_camera_track_item() override;

protected:
    step compute_step() override;
    void reset_engine() override;

private:
    std::string robot_name;
    std::optional<std::string> root_name;

    std::map<std::string, std::vector<std::tuple<std::string, std::string>>> skeleton_graph;

    std::vector<std::shared_ptr<BuilderMember>> members;
    std::vector<std::shared_ptr<BuilderConstraint>> constraints;
    std::vector<std::shared_ptr<BuilderMuscle>> muscles;

    std::shared_ptr<BuilderMember> get_member(const std::string &member_name) const;
    std::shared_ptr<BuilderConstraint> get_constraint(const std::string &constraint_name) const;
    std::shared_ptr<BuilderMuscle> get_muscle(const std::string &muscle_name) const;

    template<typename Part>
    static std::shared_ptr<Part>
    get_part(const std::string &name, const std::vector<std::shared_ptr<Part>> &vec);

    template<typename Part>
    static bool exists_part(const std::string &name, const std::vector<std::shared_ptr<Part>> &vec);
};

#endif//EVO_MOTION_BUILDER_H
