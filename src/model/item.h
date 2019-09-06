//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_ITEM_H
#define EVOMOTION_ITEM_H

#include <btBulletDynamicsCommon.h>
#include "../view/obj_mtl_vbo.h"

struct item {
	btRigidBody *m_rg_body;
	ObjMtlVBO *m_obj_mtl_vbo;
	glm::vec3 m_obj_mtl_vbo_scale;
};

btRigidBody::btRigidBodyConstructionInfo
localCreateInfo(btCollisionShape *collision_shape, glm::vec3 pos, glm::mat4 rot_mat, glm::vec3 scale, float mass);

item create_item_box(glm::vec3 pos, glm::mat4 rot_mat, glm::vec3 scale, float mass);

#endif //EVOMOTION_ITEM_H
