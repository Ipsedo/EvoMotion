//
// Created by samuel on 20/01/25.
//

#ifndef EVO_MOTION_MEMBER_H
#define EVO_MOTION_MEMBER_H

#include <any>
#include <map>
#include <string>

#include <glm/glm.hpp>

#include "../item.h"
#include "../serializer.h"
#include "../shapes.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class Member {
public:
    Member(
        const std::string &name, const ShapeKind &shape_kind, const glm::vec3 &center_pos,
        const glm::quat &rotation, const glm::vec3 &scale, const float mass, const float friction,
        const bool ignore_collision);
    explicit Member(const std::shared_ptr<AbstractDeserializer> &deserializer);
    virtual std::shared_ptr<RigidBodyItem> get_item();
    virtual std::string get_name();
    virtual ~Member();

    virtual std::shared_ptr<AbstractSerializer>
    serialize(const std::shared_ptr<AbstractSerializer> &serializer);

private:
    std::map<ShapeKind, std::string> shape_kind_to_path;
    ShapeKind shape_kind;
    std::shared_ptr<RigidBodyItem> member;
};

#endif//EVO_MOTION_MEMBER_H
