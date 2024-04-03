//
// Created by samuel on 03/04/24.
//

#include "./converter.h"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// transformation

glm::mat4 json_transformation_to_model_matrix(Json::Value transformation) {

    Json::Value rotation = transformation["rotation"];
    Json::Value translation = transformation["translation"];

    glm::vec3 position = json_vec3_to_glm_vec3(transformation["translation"]);
    glm::vec3 rotation_point = json_vec3_to_glm_vec3(rotation["point"]);
    glm::vec3 rotation_axis = json_vec3_to_glm_vec3(rotation["axis"]);
    float angle_radian = rotation["angle_radian"].asFloat();

    glm::mat4 translation_to_origin = glm::translate(glm::mat4(1.0f),
                                                     -rotation_point);
    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), angle_radian,
                                            rotation_axis);
    glm::mat4 translation_back = glm::translate(glm::mat4(1.0f),
                                                rotation_point);
    glm::mat4 translation_to_position = glm::translate(glm::mat4(1.0f),
                                                       position);

    return translation_to_position * translation_back * rotation_matrix *
           translation_to_origin;
}

glm::vec3 json_vec3_to_glm_vec3(Json::Value vec3) {
    return {
        vec3["x"].asFloat(),
        vec3["y"].asFloat(),
        vec3["z"].asFloat()
    };
}

btVector3 json_vec3_to_bt_vector3(Json::Value vec3) {
    return {
        vec3["x"].asFloat(),
        vec3["y"].asFloat(),
        vec3["z"].asFloat()
    };
}


Json::Value read_json(const std::string &json_path) {
    std::ifstream stream(json_path, std::ios::in);

    Json::Value skeleton_json;

    stream >> skeleton_json;

    return skeleton_json;
}

/*
 * GLM <-> Bullet3 conversions
 */

btVector3 glm_to_bullet(glm::vec3 v) {
    return btVector3(v.x, v.y, v.z);
}

btVector4 glm_to_bullet(glm::vec4 v) {
    return btVector4(v.x, v.y, v.z, v.w);
}

btTransform glm_to_bullet(glm::mat4 m) {
    btTransform tr;
    tr.setFromOpenGLMatrix(glm::value_ptr(m));
    return tr;
}

btQuaternion glm_to_bullet(glm::quat q) {
    return btQuaternion(q.x, q.y, q.z, q.w);
}

glm::vec3 bullet_to_glm(btVector3 v) {
    return glm::vec3(v.x(), v.y(), v.z());
}

glm::vec4 bullet_to_glm(btVector4 v) {
    return glm::vec4(v.x(), v.y(), v.z(), v.w());
}

glm::mat4 bullet_to_glm(btTransform m) {
    float tmp[16];
    m.getOpenGLMatrix(tmp);
    return glm::make_mat4(tmp);
}

glm::quat bullet_to_glm(btQuaternion q) {
    return glm::quat(q.w(), q.x(), q.y(), q.z());
}
