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

class BuilderMember : public Member {
public:
    BuilderMember(
        const std::string &name, ShapeKind shape_kind, const glm::vec3 &center_pos,
        const glm::quat &rotation, const glm::vec3 &scale, float mass, float friction,
        bool ignore_collision);

    explicit BuilderMember(const std::shared_ptr<AbstractDeserializer> &deserializer);

    void update_item(
        std::optional<glm::vec3> new_pos, std::optional<glm::quat> new_rot,
        std::optional<glm::vec3> new_scale, std::optional<float> new_friction,
        std::optional<bool> new_ignore_collision);

    void transform_item(std::optional<glm::vec3> new_pos, std::optional<glm::quat> new_rot);
};

/*
 * Builder Constraints
 */

class BuilderConstraint : public virtual Constraint {
public:
    BuilderConstraint(
        const std::string &name, const std::shared_ptr<Member> &parent,
        const std::shared_ptr<Member> &child);
    BuilderConstraint(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)> &get_member_function);
};

class BuilderHingeConstraint : public virtual HingeConstraint, public virtual BuilderConstraint {
public:
    BuilderHingeConstraint(
        const std::string &name, const std::shared_ptr<Member> &parent,
        const std::shared_ptr<Member> &child, const glm::vec3 &pivot_in_parent,
        const glm::vec3 &pivot_in_child, glm::vec3 axis_in_parent, glm::vec3 axis_in_child,
        float limit_degree_min, float limit_degree_max);

    BuilderHingeConstraint(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)> &get_member_function);
};

class BuilderFixedConstraint : public virtual FixedConstraint, public virtual BuilderConstraint {
public:
    BuilderFixedConstraint(
        const std::string &name, const std::shared_ptr<Member> &parent,
        const std::shared_ptr<Member> &child, const glm::mat4 &frame_in_parent,
        const glm::mat4 &frame_in_child);

    BuilderFixedConstraint(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)> &get_member_function);
};

/*
 * Muscles
 */

class BuilderMuscle : public Muscle {
public:
    BuilderMuscle(
        const std::string &name, float attach_mass, const glm::vec3 &attach_scale,
        const Item &item_a, const glm::vec3 &pos_in_a, const Item &item_b,
        const glm::vec3 &pos_in_b, float force, float max_speed);

    BuilderMuscle(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)> &get_member_function);
};

/*
 * Environment
 */

class RobotBuilderEnvironment : public Environment {
public:
    RobotBuilderEnvironment();

    bool set_root(const std::string &member_name);
    bool add_member(
        const std::string &member_name, glm::vec3 center_pos, glm::quat rotation, glm::vec3 scale,
        float mass, float friction);

    bool update_member(
        const std::string &member_name, std::optional<glm::vec3> new_pos,
        const std::optional<glm::quat> &new_rot, std::optional<glm::vec3> new_scale,
        std::optional<float> new_friction, std::optional<bool> new_ignore_collision);

    void save_robot(const std::filesystem::path &output_json_path, const std::string &robot_name);
    void load_robot(const std::filesystem::path &input_json_path);

    /*
     * Environment
     */

    std::vector<Item> get_items() override;
    std::vector<std::shared_ptr<Controller>> get_controllers() override;
    std::vector<int64_t> get_state_space() override;
    std::vector<int64_t> get_action_space() override;
    std::optional<Item> get_camera_track_item() override;

protected:
    step compute_step() override;
    void reset_engine() override;

private:
    std::string root_name;

    std::map<std::string, std::vector<std::string>> skeleton_graph;

    std::vector<std::shared_ptr<BuilderMember>> members;
    std::vector<std::shared_ptr<BuilderConstraint>> constraints;
    std::vector<std::shared_ptr<BuilderMuscle>> muscles;

    bool member_exists(const std::string &member_name);
    bool constraint_exists(const std::string &constraint_name);
    bool muscle_exists(const std::string &muscle_name);

    std::shared_ptr<BuilderMember> get_member(const std::string &member_name);
    std::shared_ptr<BuilderConstraint> get_constraint(const std::string &constraint_name);
    std::shared_ptr<BuilderMuscle> get_muscle(const std::string &muscle_name);

    template<typename Part>
    static std::shared_ptr<Part>
    get_part(const std::string &name, std::vector<std::shared_ptr<Part>> vec);

    template<typename Part>
    static bool exists_part(const std::string &name, std::vector<std::shared_ptr<Part>> vec);
};

#endif//EVO_MOTION_BUILDER_H
