//
// Created by samuel on 15/01/25.
//

#include <fstream>
#include <utility>

#include <evo_motion_model/converter.h>
#include <evo_motion_model/robot/builder.h>
#include <evo_motion_model/robot/skeleton.h>

#include "../json_serializer.h"
#include "../utils.h"

RobotBuilderEnvironment::RobotBuilderEnvironment(std::string robot_name)
    : Environment(1), robot_name(std::move(robot_name)), root_name(std::nullopt), skeleton_graph(),
      members(), constraints(), muscles() {}

/*
 * Members
 */

bool RobotBuilderEnvironment::add_member(
    const std::string &member_name, const ShapeKind &shape_kind, const glm::vec3 &center_pos,
    const glm::quat &rotation, const glm::vec3 &scale, const float mass, const float friction) {
    if (member_exists(member_name)) return false;

    skeleton_graph[member_name] = std::vector<std::tuple<std::string, std::string>>();

    members.push_back(std::make_shared<BuilderMember>(
        member_name, shape_kind, center_pos, rotation, scale, mass, friction, false));

    members.back()->get_item()->get_body()->setUserPointer(new std::string(member_name));

    m_world->addRigidBody(members.back()->get_item()->get_body());

    return true;
}

bool RobotBuilderEnvironment::update_member(
    const std::string &member_name, const std::optional<glm::vec3> &new_pos,
    const std::optional<glm::quat> &new_rot, const std::optional<glm::vec3> &new_scale,
    const std::optional<float> &new_friction, const std::optional<float> &new_mass,
    const std::optional<bool> &new_ignore_collision) {
    if (member_exists(member_name)) {
        std::set<std::string> updated_members;

        const auto parent_member = get_member(member_name);

        glm::mat4 old_member_model_mat = parent_member->get_item()->model_matrix_without_scale();

        m_world->removeRigidBody(parent_member->get_item()->get_body());// for scaling
        parent_member->update_item(
            new_pos, new_rot, new_scale, new_friction, new_mass, new_ignore_collision);
        updated_members.insert(member_name);
        m_world->addRigidBody(parent_member->get_item()->get_body());// for scaling

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

            m_world->removeRigidBody(curr_member->get_item()->get_body());
            curr_member->update_item(curr_tr, curr_rot);
            m_world->addRigidBody(curr_member->get_item()->get_body());

            const auto curr_new_model_mat = curr_member->get_item()->model_matrix_without_scale();

            updated_members.insert(curr_member_name);

            for (const auto &[_, child_member_name]: skeleton_graph[curr_member_name])
                if (!updated_members.contains(child_member_name))
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

    for (const auto &[key, value]: skeleton_graph)
        skeleton_graph[key] = transform_vector<
            std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>(
            value, [old_name, new_name](const auto &t) {
                const auto [c, n] = t;
                return std::tuple<std::string, std::string>(c, n == old_name ? new_name : n);
            });

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
            delete static_cast<std::string *>(m->get_item()->get_body()->getUserPointer());
            m_world->removeRigidBody(m->get_item()->get_body());
            delete m->get_item()->get_body();
            return true;
        }
        return false;
    });

    skeleton_graph.erase(member_name);

    if (root_name.has_value() && root_name.value() == member_name) root_name = std::nullopt;

    return true;
}

std::tuple<glm::vec3, glm::quat, glm::vec3>
RobotBuilderEnvironment::get_member_transform(const std::string &member_name) const {
    return decompose_model_matrix(get_member(member_name)->get_item()->model_matrix());
}

std::optional<std::string> RobotBuilderEnvironment::get_root_name() { return root_name; }

float RobotBuilderEnvironment::get_member_mass(const std::string &member_name) const {
    return get_member(member_name)->get_item()->get_body()->getMass();
}
float RobotBuilderEnvironment::get_member_friction(const std::string &member_name) const {
    return get_member(member_name)->get_item()->get_body()->getFriction();
}

int RobotBuilderEnvironment::get_members_count() const { return static_cast<int>(members.size()); }

std::vector<std::string> RobotBuilderEnvironment::get_member_names() {
    std::vector<std::string> names;
    std::ranges::transform(
        members, std::back_inserter(names), [](const auto &m) { return m->get_name(); });
    return names;
}

bool RobotBuilderEnvironment::member_exists(const std::string &member_name) const {
    return exists_part(member_name, members);
}

std::shared_ptr<BuilderMember>
RobotBuilderEnvironment::get_member(const std::string &member_name) const {
    return get_part<BuilderMember>(member_name, members);
}

bool RobotBuilderEnvironment::clone_body_part(
    const std::string &member_name, const std::string &prefix_name, const glm::vec3 &center_pos,
    const glm::quat &rotation) {
    if (!member_exists(member_name)) return false;

    std::vector<std::shared_ptr<BuilderMember>> member_to_add;
    std::vector<std::shared_ptr<BuilderConstraint>> constraint_to_add;
    std::set<std::string> constraint_original_name_to_add;

    std::queue<std::shared_ptr<BuilderMember>> queue;
    queue.emplace(get_member(member_name));

    std::set<std::string> added_members;

    while (!queue.empty()) {
        auto curr_member = queue.front();
        queue.pop();

        std::shared_ptr<JsonSerializer> serializer =
            std::make_shared<JsonSerializer>(nlohmann::json());
        auto serialized = curr_member->serialize(serializer);

        nlohmann::json content = std::any_cast<nlohmann::json>(serialized->get_data());
        content["name"] = prefix_name + content["name"].get<std::string>();

        const auto new_name = content["name"].get<std::string>();

        if (member_exists(new_name)) return false;

        std::shared_ptr<JsonDeserializer> deserializer =
            std::make_shared<JsonDeserializer>(content);

        member_to_add.push_back(std::make_shared<BuilderMember>(deserializer));
        skeleton_graph[new_name] = {};
        added_members.insert(new_name);

        for (const auto &[c, n]: skeleton_graph[curr_member->get_name()]) {
            if (!added_members.contains(prefix_name + n)) queue.emplace(get_member(n));

            if (constraint_exists(prefix_name + c)) return false;

            constraint_original_name_to_add.insert(c);
        }
    }

    for (const auto &m: member_to_add) {
        m->get_item()->get_body()->setUserPointer(new std::string(m->get_name()));
        m_world->addRigidBody(m->get_item()->get_body());
        members.push_back(m);
    }

    for (const auto &c: constraint_original_name_to_add) {
        auto constraint = get_constraint(c);

        auto serializer = std::make_shared<JsonSerializer>(nlohmann::json());
        auto serialized = constraint->serialize(serializer);

        auto content = std::any_cast<nlohmann::json>(serialized->get_data());
        content["name"] = prefix_name + content["name"].get<std::string>();

        content["parent_name"] = prefix_name + content["parent_name"].get<std::string>();
        content["child_name"] = prefix_name + content["child_name"].get<std::string>();

        auto deserializer = std::make_shared<JsonDeserializer>(content);

        std::shared_ptr<BuilderConstraint> new_constraint;
        if (deserializer->read_str("type") == "hinge")
            new_constraint = std::make_shared<BuilderHingeConstraint>(
                deserializer, [this](const auto &n) { return get_member(n); });
        else if (deserializer->read_str("type") == "fixed")
            new_constraint = std::make_shared<BuilderFixedConstraint>(
                deserializer, [this](const auto &n) { return get_member(n); });

        constraints.push_back(new_constraint);

        skeleton_graph[new_constraint->get_parent()->get_name()].emplace_back(
            new_constraint->get_name(), new_constraint->get_child()->get_name());
        skeleton_graph[new_constraint->get_child()->get_name()].emplace_back(
            new_constraint->get_name(), new_constraint->get_parent()->get_name());

        m_world->addConstraint(new_constraint->get_constraint());
    }

    return update_member(prefix_name + member_name, center_pos, rotation);
}

/*
 * Constraint
 */

bool RobotBuilderEnvironment::attach_fixed_constraint(
    const std::string &constraint_name, const std::string &parent_name,
    const std::string &child_name, const glm::vec3 &absolute_fixed_point,
    const glm::quat &absolute_rotation) {
    if (constraint_exists(constraint_name) || !member_exists(parent_name)
        || !member_exists(child_name))
        return false;

    const auto parent_model_matrix =
        get_member(parent_name)->get_item()->model_matrix_without_scale();
    const auto child_model_matrix =
        get_member(child_name)->get_item()->model_matrix_without_scale();

    const auto absolute_frame =
        glm::translate(glm::mat4(1.f), absolute_fixed_point) * glm::toMat4(absolute_rotation);
    const auto frame_in_parent = glm::inverse(parent_model_matrix) * absolute_frame;
    const auto frame_in_child = glm::inverse(child_model_matrix) * absolute_frame;

    skeleton_graph[parent_name].emplace_back(constraint_name, child_name);
    skeleton_graph[child_name].emplace_back(constraint_name, parent_name);

    constraints.push_back(std::make_shared<BuilderFixedConstraint>(
        constraint_name, get_member(parent_name), get_member(child_name), frame_in_parent,
        frame_in_child));

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

std::tuple<std::string, std::string>
RobotBuilderEnvironment::get_constraint_members(const std::string &constraint_name) {
    const auto c = get_constraint(constraint_name);
    return {c->get_parent()->get_name(), c->get_child()->get_name()};
}

ConstraintType
RobotBuilderEnvironment::get_constraint_type(const std::string &constraint_name) const {
    if (std::dynamic_pointer_cast<BuilderHingeConstraint>(get_constraint(constraint_name)))
        return HINGE;
    if (std::dynamic_pointer_cast<BuilderFixedConstraint>(get_constraint(constraint_name)))
        return FIXED;
    throw std::runtime_error("Unrecognized constraint \"" + constraint_name + "\"");
}

std::tuple<glm::vec3, glm::vec3, float, float>
RobotBuilderEnvironment::get_constraint_hinge_info(const std::string &hinge_constraint_name) const {
    const auto c =
        std::dynamic_pointer_cast<BuilderHingeConstraint>(get_constraint(hinge_constraint_name));

    const auto parent_model_mat = c->get_parent()->get_item()->model_matrix_without_scale();
    const auto bullet_constraint = dynamic_cast<btHingeConstraint *>(c->get_constraint());
    const auto frame_in_parent = bullet_to_glm(bullet_constraint->getFrameOffsetA());

    const auto absolute_frame = parent_model_mat * frame_in_parent;

    const auto pos = glm::vec3(absolute_frame[3]);
    const auto axis = glm::vec3(absolute_frame[2]);

    return std::tuple{
        pos, axis, bullet_constraint->getLowerLimit(), bullet_constraint->getUpperLimit()};
}

std::tuple<glm::vec3, glm::quat>
RobotBuilderEnvironment::get_constraint_fixed_info(const std::string &fixed_constraint_name) const {
    const auto c =
        std::dynamic_pointer_cast<BuilderFixedConstraint>(get_constraint(fixed_constraint_name));

    const auto parent_model_mat = c->get_parent()->get_item()->model_matrix_without_scale();
    const auto frame_in_parent =
        bullet_to_glm(dynamic_cast<btFixedConstraint *>(c->get_constraint())->getFrameOffsetA());
    const auto absolute_frame = parent_model_mat * frame_in_parent;

    const auto [pos, rot, _] = decompose_model_matrix(absolute_frame);

    return {pos, rot};
}

bool RobotBuilderEnvironment::update_hinge_constraint(
    const std::string &hinge_constraint_name, const std::optional<glm::vec3> &new_pos,
    const std::optional<glm::vec3> &new_axis, const std::optional<float> &new_limit_angle_min,
    const std::optional<float> &new_angle_limit_max) const {
    if (!constraint_exists(hinge_constraint_name)
        || get_constraint_type(hinge_constraint_name) != HINGE)
        return false;

    std::dynamic_pointer_cast<BuilderHingeConstraint>(get_constraint(hinge_constraint_name))
        ->update_constraint(new_pos, new_axis, new_limit_angle_min, new_angle_limit_max);

    return true;
}

bool RobotBuilderEnvironment::update_fixed_constraint(
    const std::string &fixed_constraint_name, const std::optional<glm::vec3> &new_pos,
    const std::optional<glm::quat> &new_rot) const {
    if (!constraint_exists(fixed_constraint_name)
        || get_constraint_type(fixed_constraint_name) != FIXED)
        return false;

    std::dynamic_pointer_cast<BuilderFixedConstraint>(get_constraint(fixed_constraint_name))
        ->update_constraint(new_pos, new_rot);

    return true;
}

bool RobotBuilderEnvironment::constraint_exists(const std::string &constraint_name) const {
    return exists_part(constraint_name, constraints);
}

std::shared_ptr<BuilderConstraint>
RobotBuilderEnvironment::get_constraint(const std::string &constraint_name) const {
    return get_part<BuilderConstraint>(constraint_name, constraints);
}

/*
 * Utility
 */

bool RobotBuilderEnvironment::set_root(const std::string &member_name) {
    if (member_exists(member_name)) {
        root_name = member_name;
        return true;
    }
    return false;
}

std::string RobotBuilderEnvironment::get_robot_name() { return robot_name; }

void RobotBuilderEnvironment::set_robot_name(const std::string &new_robot_name) {
    robot_name = new_robot_name;
}

bool RobotBuilderEnvironment::muscle_exists(const std::string &muscle_name) const {
    return exists_part(muscle_name, muscles);
}

template<typename Part>
bool RobotBuilderEnvironment::exists_part(
    const std::string &name, const std::vector<std::shared_ptr<Part>> &vec) {
    return std::any_of(
        vec.begin(), vec.end(), [name](const auto &p) { return p->get_name() == name; });
}

std::shared_ptr<BuilderMuscle>
RobotBuilderEnvironment::get_muscle(const std::string &muscle_name) const {
    return get_part<BuilderMuscle>(muscle_name, muscles);
}

template<typename Part>
std::shared_ptr<Part> RobotBuilderEnvironment::get_part(
    const std::string &name, const std::vector<std::shared_ptr<Part>> &vec) {
    for (const auto &o: vec)
        if (o->get_name() == name) return o;
    throw std::runtime_error("Part \"" + name + "\" not found");
}

/*
 * Ray cast
 */

std::optional<std::string> RobotBuilderEnvironment::ray_cast_member(
    const glm::vec3 &from_absolute, const glm::vec3 &to_absolute) const {
    const auto from_bullet = glm_to_bullet(from_absolute);
    const auto to_bullet = glm_to_bullet(to_absolute);

    btCollisionWorld::ClosestRayResultCallback callback(from_bullet, to_bullet);

    m_world->rayTest(from_bullet, to_bullet, callback);

    if (callback.hasHit())
        if (const auto user_ptr = callback.m_collisionObject->getUserPointer(); user_ptr) {
            if (std::string name(*static_cast<std::string *>(user_ptr)); member_exists(name))
                return name;
        }

    return std::nullopt;
}

std::optional<std::string> RobotBuilderEnvironment::ray_cast_constraint(
    const glm::vec3 &from_absolute, const glm::vec3 &to_absolute) {

    // prepare world
    std::vector<btRigidBody *> bodies_to_re_add;
    std::ranges::transform(members, std::back_inserter(bodies_to_re_add), [this](const auto &m) {
        const auto b = m->get_item()->get_body();
        m_world->removeRigidBody(b);
        return b;
    });

    for (const auto &m: muscles)
        for (const auto &b: m->get_bodies()) {
            m_world->removeRigidBody(b);
            bodies_to_re_add.push_back(b);
        }

    std::vector<btRigidBody *> tmp_bodies;
    std::ranges::transform(
        constraints, std::back_inserter(tmp_bodies),
        [this](const std::shared_ptr<BuilderConstraint> &c) {
            const auto b = c->create_fake_body();
            b->setUserPointer(new std::string(c->get_name()));
            m_world->addRigidBody(b);
            return b;
        });

    // ray cast
    const auto from_bullet = glm_to_bullet(from_absolute);
    const auto to_bullet = glm_to_bullet(to_absolute);
    std::optional<std::string> found = std::nullopt;

    btCollisionWorld::ClosestRayResultCallback callback(from_bullet, to_bullet);

    m_world->rayTest(from_bullet, to_bullet, callback);

    if (callback.hasHit())
        if (const auto user_ptr = callback.m_collisionObject->getUserPointer(); user_ptr) {
            if (std::string name(*static_cast<std::string *>(user_ptr)); constraint_exists(name))
                found = name;
        }

    // re-populate world
    for (const auto &b: tmp_bodies) {
        delete static_cast<std::string *>(b->getUserPointer());
        m_world->removeRigidBody(b);
    }

    for (const auto &b: bodies_to_re_add) m_world->addRigidBody(b);

    return found;
}

/*
 * Save / Load
 */

void RobotBuilderEnvironment::save_robot(const std::filesystem::path &output_json_path) {
    const std::vector<std::shared_ptr<Member>> members_vector(members.begin(), members.end());
    const std::vector<std::shared_ptr<Constraint>> constraints_vector(
        constraints.begin(), constraints.end());
    const std::vector<std::shared_ptr<Muscle>> muscles_vector(muscles.begin(), muscles.end());

    Skeleton skeleton(
        robot_name, root_name.has_value() ? root_name.value() : members_vector[0]->get_name(),
        members_vector, constraints_vector, muscles_vector);

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

    std::ranges::transform(
        json_members_deserializer, std::back_inserter(members),
        [this](const auto &s) -> std::shared_ptr<BuilderMember> {
            const auto m = std::make_shared<BuilderMember>(s);

            m->get_item()->get_body()->setUserPointer(new std::string(m->get_name()));

            m_world->addRigidBody(m->get_item()->get_body());
            skeleton_graph[m->get_name()] = std::vector<std::tuple<std::string, std::string>>();

            return m;
        });

    std::ranges::transform(
        json_constraints_deserializer, std::back_inserter(constraints),
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

    std::ranges::transform(
        json_muscles_deserializer, std::back_inserter(muscles),
        [this](const auto &s) -> std::shared_ptr<BuilderMuscle> {
            const auto m =
                std::make_shared<BuilderMuscle>(s, [this](const auto &n) { return get_member(n); });
            for (const auto &b: m->get_bodies()) m_world->addRigidBody(b);
            for (const auto &c: m->get_constraints()) m_world->addConstraint(c);
            return m;
        });
}

/*
 * Environment methods
 */

std::vector<std::shared_ptr<ShapeItem>> RobotBuilderEnvironment::get_draw_items() {
    std::vector<std::shared_ptr<ShapeItem>> items;

    std::ranges::transform(
        members, std::back_inserter(items), [](const auto &m) { return m->get_item(); });

    std::ranges::transform(
        constraints, std::back_inserter(items),
        [](const std::shared_ptr<BuilderConstraint> &c) { return c->get_empty_item(); });

    for (auto c: constraints)
        for (const auto &i: c->get_builder_empty_items()) items.push_back(i);

    return items;
}
std::vector<std::shared_ptr<Controller>> RobotBuilderEnvironment::get_controllers() { return {}; }
std::vector<int64_t> RobotBuilderEnvironment::get_state_space() { return {}; }
std::vector<int64_t> RobotBuilderEnvironment::get_action_space() { return {}; }
std::optional<std::shared_ptr<ShapeItem>> RobotBuilderEnvironment::get_camera_track_item() {
    if (root_name.has_value()) return get_member(root_name.value())->get_item();
    return std::nullopt;
}
step RobotBuilderEnvironment::compute_step() { return {}; }
void RobotBuilderEnvironment::reset_engine() {}
