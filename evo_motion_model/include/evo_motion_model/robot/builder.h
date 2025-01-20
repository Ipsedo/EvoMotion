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
#include "./skeleton.h"

/*
 * Parameter Member
 */

class BuilderMember : public Member {
public:
    BuilderMember(
        const std::string &name, ShapeKind shapeKind, const glm::vec3 &centerPos,
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

class BuilderConstraint : public Constraint {};

class BuilderHingeConstraint : public virtual BuilderConstraint, public virtual HingeConstraint {};

class BuilderFixedConstraint : public virtual BuilderConstraint, public virtual FixedConstraint {};

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

private:
    std::string root_name;

    std::map<std::string, std::vector<std::string>> skeleton_graph;

    std::map<std::string, std::shared_ptr<BuilderMember>> members;
    std::map<std::string, std::shared_ptr<BuilderConstraint>> constraints;

    bool member_exists(const std::string &member_name);
};

#endif//EVO_MOTION_BUILDER_H
