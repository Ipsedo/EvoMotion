//
// Created by samuel on 15/01/25.
//

#include <fstream>
#include <utility>

#include <evo_motion_model/robot/builder.h>
#include <evo_motion_model/robot/skeleton.h>

#include "../converter.h"
#include "../json_serializer.h"
#include "../utils.h"

RobotBuilderEnvironment::RobotBuilderEnvironment(std::string robot_name)
    : Environment(1), robot_name(std::move(robot_name)), root_name(), skeleton_graph(), members(),
      constraints(), muscles() {}

bool RobotBuilderEnvironment::set_root(const std::string &member_name) {
    if (member_exists(member_name)) {
        root_name = member_name;
        return true;
    }
    return false;
}

bool RobotBuilderEnvironment::add_member(
    const std::string &member_name, ShapeKind shape_kind, glm::vec3 center_pos, glm::quat rotation,
    glm::vec3 scale, float mass, float friction) {
    if (member_exists(member_name)) return false;

    skeleton_graph[member_name] = std::vector<std::tuple<std::string, std::string>>();

    members.push_back(std::make_shared<BuilderMember>(
        member_name, shape_kind, center_pos, rotation, scale, mass, friction, false));

    members.back()->get_item()->get_body()->setUserPointer(new std::string(member_name));

    return true;
}

bool RobotBuilderEnvironment::update_member(
    const std::string &member_name, std::optional<glm::vec3> new_pos,
    const std::optional<glm::quat> &new_rot, std::optional<glm::vec3> new_scale,
    std::optional<float> new_friction, std::optional<float> new_mass,
    std::optional<bool> new_ignore_collision) {

    if (member_exists(member_name)) {
        std::set<std::string> updated_members;

        const auto parent_member = get_member(member_name);

        glm::mat4 old_member_model_mat = parent_member->get_item()->model_matrix_without_scale();

        parent_member->update_item(
            new_pos, new_rot, new_scale, new_friction, new_mass, new_ignore_collision);
        updated_members.insert(member_name);

        glm::mat4 new_member_model_mat = parent_member->get_item()->model_matrix_without_scale();

        std::queue<std::tuple<glm::mat4, glm::mat4, std::string>> queue;
        for (const auto &[_, m_n]: skeleton_graph[member_name])
            queue.emplace(old_member_model_mat, new_member_model_mat, m_n);

        while (!queue.empty()) {
            const auto &[parent_old_model_mat, parent_new_model_mat, curr_member_name] =
                queue.front();

            const auto curr_member = get_member(curr_member_name);
            const auto curr_old_model_mat = curr_member->get_item()->model_matrix_without_scale();

            const auto in_old_parent_space =
                glm::inverse(parent_old_model_mat) * curr_old_model_mat;
            const auto in_new_parent_space = parent_new_model_mat * in_old_parent_space;

            const auto [curr_tr, curr_rot, curr_scale] =
                decompose_model_matrix(in_new_parent_space);

            curr_member->update_item(curr_tr, curr_rot);
            const auto curr_new_model_mat = curr_member->get_item()->model_matrix_without_scale();

            updated_members.insert(curr_member_name);

            for (const auto &[_, child_member_name]: skeleton_graph[curr_member_name])
                if (updated_members.find(child_member_name) == updated_members.end())
                    queue.emplace(curr_old_model_mat, curr_new_model_mat, child_member_name);

            queue.pop();
        }
        return true;
    }
    return false;
}

bool RobotBuilderEnvironment::rename_member(
    const std::string &old_name, const std::string &new_name) {
    if (member_exists(new_name) || !member_exists(old_name)) return false;

    get_member(old_name)->get_item()->rename(new_name);

    if (root_name == old_name) root_name = new_name;

    auto children = skeleton_graph.extract(old_name);
    children.key() = new_name;
    skeleton_graph.insert(std::move(children));

    for (const auto [key, value]: skeleton_graph)
        skeleton_graph[key] = transform_vector<
            std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>(
            value, [old_name, new_name](const auto &t) {
                const auto [c, n] = t;
                return std::tuple<std::string, std::string>(c, n == old_name ? new_name : n);
            });

    return true;
}

bool RobotBuilderEnvironment::attach_fixed_constraint(
    const std::string &constraint_name, const std::string &parent_name,
    const std::string &child_name, const glm::vec3 &absolute_fixed_point) {
    if (constraint_exists(constraint_name) || !member_exists(parent_name)
        || !member_exists(child_name))
        return false;

    const auto parent_model_matrix =
        get_member(parent_name)->get_item()->model_matrix_without_scale();
    const auto child_model_matrix =
        get_member(child_name)->get_item()->model_matrix_without_scale();

    const auto absolute_frame = glm::translate(glm::mat4(1.f), absolute_fixed_point);
    const auto frame_in_parent = glm::inverse(parent_model_matrix) * absolute_frame;
    const auto frame_in_child = glm::inverse(child_model_matrix) * absolute_frame;

    skeleton_graph[parent_name].emplace_back(constraint_name, child_name);
    skeleton_graph[child_name].emplace_back(constraint_name, parent_name);

    constraints.push_back(std::make_shared<BuilderFixedConstraint>(
        constraint_name, get_member(parent_name), get_member(child_name), frame_in_parent,
        frame_in_child));

    return true;
}

bool RobotBuilderEnvironment::remove_member(const std::string &member_name) {
    if (!member_exists(member_name)) return false;

    std::vector<std::string> constraints_to_remove;
    for (const auto &[constraint_name, curr_member_name]: skeleton_graph[member_name])
        constraints_to_remove.push_back(constraint_name);

    for (const auto &constraint_name: constraints_to_remove) remove_constraint(constraint_name);

    std::erase_if(members, [member_name, this](const auto &m) {
        if (m->get_name() == member_name) {
            delete (std::string *) m->get_item()->get_body()->getUserPointer();
            m_world->removeRigidBody(m->get_item()->get_body());
            delete m->get_item()->get_body();
            return true;
        }
        return false;
    });

    skeleton_graph.erase(member_name);

    return true;
}

bool RobotBuilderEnvironment::remove_constraint(const std::string &constraint_name) {
    if (!constraint_exists(constraint_name)) return false;

    for (auto [member, children]: skeleton_graph) {
        std::erase_if(children, [constraint_name](const auto &t) {
            const auto &[curr_constraint_name, _] = t;
            return curr_constraint_name == constraint_name;
        });
        skeleton_graph[member] = children;
    }

    std::erase_if(constraints, [constraint_name, this](const auto &c) {
        if (c->get_name() == constraint_name) {
            m_world->removeConstraint(c->get_constraint());
            delete c->get_constraint();
            return true;
        }
        return false;
    });

    return true;
}

bool RobotBuilderEnvironment::member_exists(const std::string &member_name) {
    return exists_part(member_name, members);
}

bool RobotBuilderEnvironment::constraint_exists(const std::string &constraint_name) {
    return exists_part(constraint_name, constraints);
}

bool RobotBuilderEnvironment::muscle_exists(const std::string &muscle_name) {
    return exists_part(muscle_name, muscles);
}

template<typename Part>
bool RobotBuilderEnvironment::exists_part(
    const std::string &name, std::vector<std::shared_ptr<Part>> vec) {
    return std::any_of(
        vec.begin(), vec.end(), [name](const auto &p) { return p->get_name() == name; });
}

std::shared_ptr<BuilderMember> RobotBuilderEnvironment::get_member(const std::string &member_name) {
    return get_part<BuilderMember>(member_name, members);
}

std::shared_ptr<BuilderConstraint>
RobotBuilderEnvironment::get_constraint(const std::string &constraint_name) {
    return get_part<BuilderConstraint>(constraint_name, constraints);
}

std::shared_ptr<BuilderMuscle> RobotBuilderEnvironment::get_muscle(const std::string &muscle_name) {
    return get_part<BuilderMuscle>(muscle_name, muscles);
}

template<typename Part>
std::shared_ptr<Part>
RobotBuilderEnvironment::get_part(const std::string &name, std::vector<std::shared_ptr<Part>> vec) {
    for (const auto &o: vec)
        if (o->get_name() == name) return o;
    throw std::runtime_error("Part \"" + name + "\" not found");
}

std::optional<std::string> RobotBuilderEnvironment::ray_cast_member(
    const glm::vec3 &from_absolute, const glm::vec3 &to_absolute) {
    const auto from_bullet = glm_to_bullet(from_absolute);
    const auto to_bullet = glm_to_bullet(to_absolute);

    btCollisionWorld::ClosestRayResultCallback callback(from_bullet, to_bullet);

    m_world->rayTest(from_bullet, to_bullet, callback);

    if (callback.hasHit())
        if (const auto user_ptr = callback.m_collisionObject->getUserPointer(); user_ptr) {
            const std::string name(*static_cast<std::string *>(user_ptr));
            if (member_exists(name)) return name;
        }

    return std::nullopt;
}

void RobotBuilderEnvironment::save_robot(const std::filesystem::path &output_json_path) {

    std::vector<std::shared_ptr<Member>> members_vector(members.begin(), members.end());
    std::vector<std::shared_ptr<Constraint>> constraints_vector(
        constraints.begin(), constraints.end());
    std::vector<std::shared_ptr<Muscle>> muscles_vector(muscles.begin(), muscles.end());

    Skeleton skeleton(robot_name, root_name, members_vector, constraints_vector, muscles_vector);

    const auto json_serializer = std::make_shared<JsonSerializer>();
    skeleton.serialize(json_serializer);
    json_serializer->to_file(output_json_path);
}

void RobotBuilderEnvironment::load_robot(const std::filesystem::path &input_json_path) {
    const auto json_deserializer = std::make_shared<JsonDeserializer>(input_json_path);

    const auto json_members_deserializer = json_deserializer->read_array("members");
    const auto json_constraints_deserializer = json_deserializer->read_array("constraints");
    const auto json_muscles_deserializer = json_deserializer->read_array("muscles");

    root_name = json_deserializer->read_str("root_name");
    robot_name = json_deserializer->read_str("robot_name");

    skeleton_graph.clear();
    members.clear();
    constraints.clear();
    muscles.clear();

    std::transform(
        json_members_deserializer.begin(), json_members_deserializer.end(),
        std::back_inserter(members), [this](const auto &s) -> std::shared_ptr<BuilderMember> {
            const auto m = std::make_shared<BuilderMember>(s);

            m->get_item()->get_body()->setUserPointer(new std::string(m->get_name()));

            m_world->addRigidBody(m->get_item()->get_body());
            skeleton_graph[m->get_name()] = std::vector<std::tuple<std::string, std::string>>();

            return m;
        });

    std::transform(
        json_constraints_deserializer.begin(), json_constraints_deserializer.end(),
        std::back_inserter(constraints),
        [this](const auto &s) -> std::shared_ptr<BuilderConstraint> {
            std::shared_ptr<BuilderConstraint> c;
            if (s->read_str("type") == "hinge")
                c = std::make_shared<BuilderHingeConstraint>(
                    s, [this](const auto &n) { return get_member(n); });
            else if (s->read_str("type") == "fixed")
                c = std::make_shared<BuilderFixedConstraint>(
                    s, [this](const auto &n) { return get_member(n); });
            else
                throw std::runtime_error("Unknown constraint type \"" + s->read_str("type") + "\"");

            m_world->addConstraint(c->get_constraint());

            skeleton_graph[c->get_parent()->get_name()].emplace_back(
                c->get_name(), c->get_child()->get_name());
            skeleton_graph[c->get_child()->get_name()].emplace_back(
                c->get_name(), c->get_parent()->get_name());

            return c;
        });

    std::transform(
        json_muscles_deserializer.begin(), json_muscles_deserializer.end(),
        std::back_inserter(muscles), [this](const auto &s) -> std::shared_ptr<BuilderMuscle> {
            const auto m =
                std::make_shared<BuilderMuscle>(s, [this](const auto &n) { return get_member(n); });
            for (const auto &b: m->get_bodies()) m_world->addRigidBody(b);
            for (const auto &c: m->get_constraints()) m_world->addConstraint(c);
            return m;
        });
}

std::string RobotBuilderEnvironment::get_robot_name() { return robot_name; }
void RobotBuilderEnvironment::set_robot_name(const std::string &new_robot_name) {
    robot_name = new_robot_name;
}

std::tuple<glm::vec3, glm::quat, glm::vec3>
RobotBuilderEnvironment::get_member_transform(const std::string &member_name) {
    return decompose_model_matrix(get_member(member_name)->get_item()->model_matrix());
}

std::string RobotBuilderEnvironment::get_root_name() { return root_name; }

float RobotBuilderEnvironment::get_member_mass(const std::string &member_name) {
    return get_member(member_name)->get_item()->get_body()->getMass();
}
float RobotBuilderEnvironment::get_member_friction(const std::string &member_name) {
    return get_member(member_name)->get_item()->get_body()->getFriction();
}

int RobotBuilderEnvironment::get_members_count() { return static_cast<int>(members.size()); }

std::vector<std::string> RobotBuilderEnvironment::get_member_names() {
    std::vector<std::string> names;
    std::transform(members.begin(), members.end(), std::back_inserter(names), [](const auto &m) {
        return m->get_name();
    });
    return names;
}

/*
 * Environment methods
 */

std::vector<std::shared_ptr<AbstractItem>> RobotBuilderEnvironment::get_draw_items() {
    std::vector<std::shared_ptr<AbstractItem>> items;

    std::transform(members.begin(), members.end(), std::back_inserter(items), [](const auto &m) {
        return m->get_item();
    });

    std::transform(
        constraints.begin(), constraints.end(), std::back_inserter(items),
        [](const std::shared_ptr<BuilderConstraint> &c) { return c->get_empty_item(); });

    return items;
}
std::vector<std::shared_ptr<Controller>> RobotBuilderEnvironment::get_controllers() {
    return std::vector<std::shared_ptr<Controller>>();
}
std::vector<int64_t> RobotBuilderEnvironment::get_state_space() { return std::vector<int64_t>(); }
std::vector<int64_t> RobotBuilderEnvironment::get_action_space() { return std::vector<int64_t>(); }
std::optional<std::shared_ptr<AbstractItem>> RobotBuilderEnvironment::get_camera_track_item() {
    if (root_name != "") return get_member(root_name)->get_item();
    return std::nullopt;
}
step RobotBuilderEnvironment::compute_step() { return step(); }
void RobotBuilderEnvironment::reset_engine() {}
