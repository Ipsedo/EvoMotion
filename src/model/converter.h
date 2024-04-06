//
// Created by samuel on 03/04/24.
//

#ifndef EVO_MOTION_CONVERTER_H
#define EVO_MOTION_CONVERTER_H

#include <nlohmann/json.hpp>

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>


/*
 * JSON stuff
 */

nlohmann::json read_json(const std::string &json_path);

glm::mat4 json_transformation_to_model_matrix(nlohmann::json transformation);

glm::vec3 json_vec3_to_glm_vec3(nlohmann::json vec3);

btVector3 json_vec3_to_bt_vector3(nlohmann::json vec3);

/*
 * GLM <-> Bullet conversions
 */

btVector3 glm_to_bullet(glm::vec3 v);

btVector4 glm_to_bullet(glm::vec4 v);

btTransform glm_to_bullet(glm::mat4 m);

btQuaternion glm_to_bullet(glm::quat q);

glm::vec3 bullet_to_glm(btVector3 v);

glm::vec4 bullet_to_glm(btVector4 v);

glm::mat4 bullet_to_glm(const btTransform &m);

glm::quat bullet_to_glm(btQuaternion m);

#endif//EVO_MOTION_CONVERTER_H
