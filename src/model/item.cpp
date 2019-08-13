//
// Created by samuel on 11/08/19.
//

#include <glm/gtc/quaternion.hpp>
#include "item.h"
#include "../utils/res.h"

btRigidBody::btRigidBodyConstructionInfo
localCreateInfo(btCollisionShape *shape, glm::vec3 pos, glm::mat4 rot_mat, glm::vec3 scale, float mass) {

    btTransform bt_transform;
    bt_transform.setIdentity();
    bt_transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    glm::quat tmp = glm::quat_cast(rot_mat);
    bt_transform.setRotation(btQuaternion(tmp.x, tmp.y, tmp.z, tmp.w));

    btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool is_dynamic = (mass != 0.f);

    btVector3 local_inertia(0, 0, 0);
    if (is_dynamic)
        shape->calculateLocalInertia(mass, local_inertia);

    auto *m_motion_state = new btDefaultMotionState(bt_transform);

    return {mass, m_motion_state, shape, local_inertia};
}

item create_item_box(glm::vec3 pos, glm::mat4 rot_mat, glm::vec3 scale, float mass) {
    auto box_shape = new btBoxShape(btVector3(scale.x, scale.y, scale.z));
    auto rg_body = new btRigidBody(localCreateInfo(box_shape, pos, rot_mat, scale, mass));
    auto obj_mtl_vbo = new ObjMtlVBO(
            get_res_folder() + EVOMOTION_SEP + "obj" + EVOMOTION_SEP + "cube.obj",
            get_res_folder() + EVOMOTION_SEP + "obj" + EVOMOTION_SEP + "cube.mtl",
            true);
    return {rg_body, obj_mtl_vbo, scale};
}
