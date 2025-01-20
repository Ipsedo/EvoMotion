//
// Created by samuel on 20/01/25.
//

#ifndef EVO_MOTION_CONSTRAINT_H
#define EVO_MOTION_CONSTRAINT_H

#include <functional>
#include <memory>

#include "./member.h"

class Constraint {
public:
    Constraint(const std::shared_ptr<NewMember> &parent, const std::shared_ptr<NewMember> &child);
    Constraint(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<NewMember>(std::string)> &get_member_function);

    virtual btTypedConstraint *get_constraint() = 0;
    virtual ~Constraint();

    std::shared_ptr<NewMember> get_parent();
    std::shared_ptr<NewMember> get_child();

    virtual std::shared_ptr<AbstractSerializer<std::any>>
    serialize(const std::shared_ptr<AbstractSerializer<std::any>> &serializer);

private:
    std::shared_ptr<NewMember> parent;
    std::shared_ptr<NewMember> child;
};

/*
 * Constraints
 */

class HingeConstraint : public Constraint {
public:
    HingeConstraint(
        const std::shared_ptr<NewMember> &parent, const std::shared_ptr<NewMember> &child,
        glm::mat4 frame_in_parent, glm::mat4 frame_in_child, float limit_degree_min,
        float limit_degree_max);
    HingeConstraint(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<NewMember>(std::string)> &get_member_function);

    btTypedConstraint *get_constraint() override;

    std::shared_ptr<AbstractSerializer<std::any>>
    serialize(const std::shared_ptr<AbstractSerializer<std::any>> &serializer) override;

protected:
    btHingeConstraint *constraint;
};

/*
 * Fixed Constraint
 */

class FixedConstraint : public Constraint {
public:
    FixedConstraint(
        const std::shared_ptr<NewMember> &parent, const std::shared_ptr<NewMember> &child,
        glm::mat4 attach_in_parent, glm::mat4 attach_in_child);
    FixedConstraint(
        const std::shared_ptr<AbstractDeserializer> &deserialized,
        const std::function<std::shared_ptr<NewMember>(const std::string &)> &get_member_function);

    btTypedConstraint *get_constraint() override;

    std::shared_ptr<AbstractSerializer<std::any>>
    serialize(const std::shared_ptr<AbstractSerializer<std::any>> &serializer) override;

protected:
    btFixedConstraint *constraint;
};

#endif//EVO_MOTION_CONSTRAINT_H
