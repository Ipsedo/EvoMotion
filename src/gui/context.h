//
// Created by samuel on 06/02/25.
//

#ifndef EVO_MOTION_CONTEXT_H
#define EVO_MOTION_CONTEXT_H

#include <memory>
#include <optional>

#include <evo_motion_model/robot/builder.h>

template<typename T>
class Option {
public:
    Option() : value(std::nullopt) {}
    bool is_set() const { return value.has_value(); }
    T get() const { return value.value(); }
    void set(const T &new_value) { value = new_value; }
    void release() { value = std::nullopt; }

private:
    std::optional<T> value;
};

class AppContext {
public:
    explicit AppContext();

    /*
     * Builder context function
     */

    Option<std::string> focused_member;
    Option<std::string> focused_constraint;

    Option<std::shared_ptr<RobotBuilderEnvironment>> builder_env;

    Option<std::string> constraint_parent;
    Option<std::string> constraint_child;

    void hide_members(bool hidden);
    [[nodiscard]] bool are_members_hidden() const;

    void hide_constraints(bool hidden);
    [[nodiscard]] bool are_constraints_hidden() const;

private:
    bool members_hidden;
    bool constraints_hidden;
};

#endif//EVO_MOTION_CONTEXT_H
