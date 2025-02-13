//
// Created by samuel on 03/04/24.
//

#include <fstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/converter.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <bitset>

#include <glm/gtx/matrix_decompose.hpp>

/*
 * JSON stuff
 */

glm::mat4 json_transformation_to_model_matrix(nlohmann::json transformation) {

    nlohmann::json rotation = transformation["rotation"];

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
 * GLM stuff
 */

std::tuple<glm::vec3, glm::quat, glm::vec3> decompose_model_matrix(const glm::mat4 &model_matrix) {
    glm::vec3 curr_scale;
    glm::quat curr_rotation;
    glm::vec3 curr_translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(model_matrix, curr_scale, curr_rotation, curr_translation, skew, perspective);
    return {curr_translation, curr_rotation, curr_scale};
}

/*
 * GLM to JSON
 */

nlohmann::json model_matrix_to_json_transformation(const glm::mat4 &model_matrix) {

    const auto [curr_translation, curr_rotation, curr_scale] = decompose_model_matrix(model_matrix);

    return {
        {"translation", vec3_to_json(curr_translation)},
        {"rotation",
         {{"point", vec3_to_json(glm::vec3(0.f))},
          {"axis", vec3_to_json(glm::eulerAngles(curr_rotation))},
          {"angle_degree", glm::angle(curr_rotation)}}}};
}

nlohmann::json vec3_to_json(glm::vec3 vec) { return {{"x", vec.x}, {"y", vec.y}, {"z", vec.z}}; }

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

glm::quat axis_angle_to_quat(const glm::vec3 &axis, const float angle) {
    return glm::angleAxis(angle, axis);
}

std::tuple<glm::vec3, float> quat_to_axis_angle(const glm::quat &q) {
    return {glm::axis(q), glm::angle(q)};
}

/*
 * Binary conversion
 */

std::string float_to_binary_string(float f) {
    union {
        float v_f;
        uint32_t bits;
    } data{};
    data.v_f = f;

    return std::bitset<32>(data.bits).to_string();
}

float binary_string_to_float(const std::string &s) {
    union {
        uint32_t bits;
        float output;
    } data{};

    data.bits = std::bitset<32>(s).to_ulong();

    return data.output;
}
