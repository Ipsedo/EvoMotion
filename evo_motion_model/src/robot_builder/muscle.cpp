//
// Created by samuel on 21/01/25.
//

#include <evo_motion_model/robot/builder.h>

BuilderMuscle::BuilderMuscle(
    const std::string &name, float attach_mass, const glm::vec3 &attach_scale,
    const std::shared_ptr<RigidBodyItem> &item_a, const glm::vec3 &pos_in_a,
    const std::shared_ptr<RigidBodyItem> &item_b, const glm::vec3 &pos_in_b, float force,
    float max_speed)
    : Muscle(
          name, attach_mass, attach_scale, item_a, pos_in_a, item_b, pos_in_b, force, max_speed) {}

BuilderMuscle::BuilderMuscle(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<Member>(std::string)> &get_member_function)
    : Muscle(deserializer, get_member_function) {}
