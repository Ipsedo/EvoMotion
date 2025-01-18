//
// Created by samuel on 15/01/25.
//

#ifndef EVO_MOTION_JSON_SKELETON_H
#define EVO_MOTION_JSON_SKELETON_H

#include <nlohmann/json.hpp>

#include <evo_motion_model/skeleton.h>

class JsonMember final : public AbstractMember {
public:
    JsonMember(const Item &parent, const nlohmann::json &json_member);

    JsonMember(
        const std::string &name, const glm::mat4 &parent_model_matrix,
        nlohmann::json json_member_input);

    Item get_item() override;

    std::vector<std::shared_ptr<AbstractConstraint>> get_children() override;

protected:
    nlohmann::json json_member;
    glm::mat4 model_matrix;
    std::map<std::string, std::string> shape_to_path;
    std::shared_ptr<ObjShape> obj_shape;
    Item member;
};

class JsonHingeConstraint final : public AbstractConstraint {
public:
    JsonHingeConstraint(const Item &parent, const nlohmann::json &hinge);

    btTypedConstraint *get_constraint() override;

    std::shared_ptr<AbstractMember> get_child() override;

    ~JsonHingeConstraint() override;

private:
    std::shared_ptr<JsonMember> child;
    btHingeConstraint *hinge_constraint;
};

class JsonFixedConstraint final : public AbstractConstraint {
public:
    JsonFixedConstraint(const Item &parent, const nlohmann::json &fixed);

    btTypedConstraint *get_constraint() override;

    std::shared_ptr<AbstractMember> get_child() override;

    ~JsonFixedConstraint() override;

private:
    std::shared_ptr<JsonMember> child;
    btFixedConstraint *fixed_constraint;
};

class JsonSkeleton : public Skeleton {
public:
    JsonSkeleton(
        const std::string &json_path, const std::string &root_name, glm::mat4 model_matrix);
};

#endif//EVO_MOTION_JSON_SKELETON_H
