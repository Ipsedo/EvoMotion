//
// Created by samuel on 03/04/24.
//

#include "./converter.h"

#include <filesystem>
#include <fstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "./constants.h"

/*
 * JSON stuff
 */

glm::mat4 json_transformation_to_model_matrix(nlohmann::json transformation) {

    nlohmann::json rotation = transformation["rotation"];
    nlohmann::json translation = transformation["translation"];

    const glm::vec3 position = json_vec3_to_glm_vec3(transformation["translation"]);
    const glm::vec3 rotation_point = json_vec3_to_glm_vec3(rotation["point"]);
    const glm::vec3 rotation_axis = json_vec3_to_glm_vec3(rotation["axis"]);
    const float angle_radian = M_PI * rotation["angle_degree"].get<float>() / 180.f;

    const glm::mat4 translation_to_origin = glm::translate(glm::mat4(1.0f), -rotation_point);
    const glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), angle_radian, rotation_axis);
    const glm::mat4 translation_back = glm::translate(glm::mat4(1.0f), rotation_point);
    const glm::mat4 translation_to_position = glm::translate(glm::mat4(1.0f), position);

    return translation_to_position * translation_back * rotation_matrix * translation_to_origin;
}

glm::vec3 json_vec3_to_glm_vec3(nlohmann::json vec3) {
    return {vec3["x"].get<float>(), vec3["y"].get<float>(), vec3["z"].get<float>()};
}

btVector3 json_vec3_to_bt_vector3(nlohmann::json vec3) {
    return {vec3["x"].get<float>(), vec3["y"].get<float>(), vec3["z"].get<float>()};
}

nlohmann::json read_json(const std::string &json_path) {
    //std::filesystem::path resources_path(RESOURCES_PATH);
    std::ifstream stream(json_path, std::ios::in);

    nlohmann::json json = nlohmann::json::parse(stream);

    return json;
}

/*
 * GLM <-> Bullet3 conversions
 */

btVector3 glm_to_bullet(glm::vec3 v) { return {v.x, v.y, v.z}; }

btVector4 glm_to_bullet(glm::vec4 v) { return {v.x, v.y, v.z, v.w}; }

btTransform glm_to_bullet(glm::mat4 m) {
    btTransform tr;
    tr.setFromOpenGLMatrix(glm::value_ptr(m));
    return tr;
}

btQuaternion glm_to_bullet(glm::quat q) { return {q.x, q.y, q.z, q.w}; }

glm::vec3 bullet_to_glm(const btVector3 v) { return {v.x(), v.y(), v.z()}; }

glm::vec4 bullet_to_glm(const btVector4 v) { return {v.x(), v.y(), v.z(), v.w()}; }

glm::mat4 bullet_to_glm(const btTransform &m) {
    float tmp[16];
    m.getOpenGLMatrix(tmp);
    return glm::make_mat4(tmp);
}

glm::quat bullet_to_glm(const btQuaternion q) { return {q.w(), q.x(), q.y(), q.z()}; }