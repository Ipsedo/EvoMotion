//
// Created by samuel on 20/01/25.
//

#include <evo_motion_model/converter.h>
#include <evo_motion_model/robot/member.h>

/*
 * Member
 */

Member::~Member() = default;

Member::Member(
    const std::string &name, const ShapeKind &shape_kind, const glm::vec3 &center_pos,
    const glm::quat &rotation, const glm::vec3 &scale, const float mass, const float friction,
    const bool ignore_collision)
    : shape_kind_to_path(
          {{SPHERE, "./resources/obj/sphere.obj"},
           {CUBE, "./resources/obj/cube.obj"},
           {CYLINDER, "./resources/obj/cylinder.obj"},
           {FEET, "./resources/obj/feet.obj"}}),
      shape_kind(shape_kind),
      member(std::make_shared<RigidBodyItem>(
          name, std::make_shared<ObjShape>(shape_kind_to_path[shape_kind]),
          glm::translate(glm::mat4(1.f), center_pos) * glm::mat4_cast(rotation), scale, mass,
          SPECULAR)) {

    member->get_body()->setFriction(friction);

    if (ignore_collision)
        member->get_body()->setCollisionFlags(
            member->get_body()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
}
Member::Member(const std::shared_ptr<AbstractDeserializer> &deserializer)
    : Member(
          deserializer->read_str("name"), deserializer->read_shape_kind("shape"),
          deserializer->read_vec3("translation"), deserializer->read_quat("rotation"),
          deserializer->read_vec3("scale"), deserializer->read_float("mass"),
          deserializer->read_float("friction"), deserializer->read_bool("ignore_collision")) {}

std::shared_ptr<RigidBodyItem> Member::get_item() { return member; }

std::string Member::get_name() { return get_item()->get_name(); }

std::shared_ptr<AbstractSerializer>
Member::serialize(const std::shared_ptr<AbstractSerializer> &serializer) {
    auto serializer_member = serializer->new_object();

    serializer_member->write_str("name", get_item()->get_name());
    serializer_member->write_shape_kind("shape", shape_kind);

    auto [translation, rotation, scale] = decompose_model_matrix(get_item()->model_matrix());
    serializer_member->write_vec3("translation", translation);
    serializer_member->write_vec3("scale", scale);
    serializer_member->write_quat("rotation", rotation);

    serializer_member->write_float("mass", get_item()->get_body()->getMass());
    serializer_member->write_float("friction", get_item()->get_body()->getFriction());
    serializer_member->write_bool(
        "ignore_collision",
        get_item()->get_body()->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE);

    return serializer_member;
}