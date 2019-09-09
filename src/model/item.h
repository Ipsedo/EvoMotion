//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_ITEM_H
#define EVOMOTION_ITEM_H

#include <btBulletDynamicsCommon.h>
#include "../view/obj_mtl_vbo.h"

/**
 * The base struct for all items
 * Contains :
 * - a Bullet physical object (btRigidBody)
 * - a OpenGL graphical object (ObjMtlVBO)
 * - a glm::vec3 scale
 */
struct item {
	btRigidBody *m_rg_body;
	ObjMtlVBO *m_obj_mtl_vbo;
	glm::vec3 m_obj_mtl_vbo_scale;
};

/**
 * Compute Bullet rigidbody infos
 * @param collision_shape The shape of the futur rigidbody
 * @param pos The intial position
 * @param rot_mat The initial rotation matrix
 * @param scale The object scale
 * @param mass The object mass
 * @return
 */
btRigidBody::btRigidBodyConstructionInfo
localCreateInfo(btCollisionShape *collision_shape, glm::vec3 pos, glm::mat4 rot_mat, glm::vec3 scale, float mass);

/**
 * Create a item box
 * @param pos The box position
 * @param rot_mat The box rotation matrix
 * @param scale The box scale
 * @param mass The box mass
 * @return The item box (physical and graphical object)
 */
item create_item_box(glm::vec3 pos, glm::mat4 rot_mat, glm::vec3 scale, float mass);

#endif //EVOMOTION_ITEM_H
