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

/*
 * Parameter Member
 */

class BuilderMember : public NewMember {};

class BuilderConstraint : public virtual Constraint {};

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
        const std::string &member_name, glm::vec3 center_pos, glm::quat rotation, glm::vec3 scale,
        float mass, float friction);

    void save_robot(const std::filesystem::path &output_json_path);
    void load_robot(const std::filesystem::path &input_json_path);

private:
    std::string root_name;

    std::map<std::string, std::string> skeleton_graph;

    std::vector<std::shared_ptr<BuilderMember>> members;
    std::vector<std::shared_ptr<BuilderConstraint>> constraints;

    bool member_exists(const std::string &member_name);
};

#endif//EVO_MOTION_BUILDER_H
