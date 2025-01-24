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
    Constraint(
        std::string name, const std::shared_ptr<Member> &parent,
        const std::shared_ptr<Member> &child);
    Constraint(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)> &get_member_function);

    virtual btTypedConstraint *get_constraint() = 0;
    virtual ~Constraint();

    std::string get_name();
    std::shared_ptr<Member> get_parent();
    std::shared_ptr<Member> get_child();

    virtual std::shared_ptr<AbstractSerializer>
    serialize(const std::shared_ptr<AbstractSerializer> &serializer);

private:
    std::string name;

    std::shared_ptr<Member> parent;
    std::shared_ptr<Member> child;
};

/*
 * Constraints
 */

class HingeConstraint : public virtual Constraint {
public:
    HingeConstraint(
        const std::string &name, const std::shared_ptr<Member> &parent,
        const std::shared_ptr<Member> &child, const glm::vec3 &pivot_in_parent,
        const glm::vec3 &pivot_in_child, glm::vec3 axis_in_parent, glm::vec3 axis_in_child,
        float limit_degree_min, float limit_degree_max);
    HingeConstraint(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)> &get_member_function);

    btTypedConstraint *get_constraint() override;

    std::shared_ptr<AbstractSerializer>
    serialize(const std::shared_ptr<AbstractSerializer> &serializer) override;

protected:
    btHingeConstraint *constraint;

    float min_limit_degree;
    float max_limit_degree;
};

/*
 * Fixed Constraint
 */

class FixedConstraint : public virtual Constraint {
public:
    FixedConstraint(
        const std::string &name, const std::shared_ptr<Member> &parent,
        const std::shared_ptr<Member> &child, const glm::mat4 &attach_in_parent,
        const glm::mat4 &attach_in_child);
    FixedConstraint(
        const std::shared_ptr<AbstractDeserializer> &deserialized,
        const std::function<std::shared_ptr<Member>(const std::string &)> &get_member_function);

    btTypedConstraint *get_constraint() override;

    std::shared_ptr<AbstractSerializer>
    serialize(const std::shared_ptr<AbstractSerializer> &serializer) override;

protected:
    btFixedConstraint *constraint;
};

#endif//EVO_MOTION_CONSTRAINT_H
