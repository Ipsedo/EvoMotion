//
// Created by samuel on 20/01/25.
//

#ifndef EVO_MOTION_MEMBER_H
#define EVO_MOTION_MEMBER_H

#include <any>
#include <map>
#include <string>

#include <glm/glm.hpp>

#include "./item.h"
#include "./serializer.h"
#include "./shapes.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class NewMember {
public:
    NewMember(
        const std::string &name, ShapeKind shape_kind, glm::vec3 center_pos, glm::quat rotation,
        glm::vec3 scale, float mass, float friction, bool ignore_collision);
    NewMember(const std::shared_ptr<AbstractDeserializer> &deserializer);
    virtual Item get_item();
    virtual ~NewMember();

    virtual std::shared_ptr<AbstractSerializer<std::any>>
    serialize(const std::shared_ptr<AbstractSerializer<std::any>> &serializer);

private:
    std::map<ShapeKind, std::string> shape_kind_to_path;
    ShapeKind shape_kind;
    Item member;
};

#endif//EVO_MOTION_MEMBER_H
