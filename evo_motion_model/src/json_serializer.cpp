//
// Created by samuel on 20/01/25.
//

#include "./json_serializer.h"

#include <fstream>
#include <ostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "./utils.h"

/*
 * Serializer
 */

JsonSerializer::JsonSerializer() : JsonSerializer({}) {}
JsonSerializer::JsonSerializer(nlohmann::json content)
    : content(content),
      shape_kind_to_str(
          {{SPHERE, "sphere"}, {CYLINDER, "cylinder"}, {CUBE, "cube"}, {FEET, "feet"}}) {}

void JsonSerializer::write_vec3(const std::string &key, glm::vec3 vec) {
    auto vec_json = new_object();

    vec_json->write_float("x", vec.x);
    vec_json->write_float("y", vec.y);
    vec_json->write_float("z", vec.z);

    write_object(key, vec_json);
}

void JsonSerializer::write_quat(const std::string &key, glm::quat quat) {
    auto quat_json = new_object();

    new_object()->write_float("x", quat.x);
    new_object()->write_float("y", quat.y);
    new_object()->write_float("z", quat.z);
    new_object()->write_float("w", quat.w);

    write_object(key, quat_json);
}

void JsonSerializer::write_bool(const std::string &key, bool value) { write_impl(key, value); }

void JsonSerializer::write_mat4(const std::string &key, glm::mat4 mat) {
    std::vector<float> mat_array;
    float *mat_ptr = glm::value_ptr(mat);
    for (int i = 0; i < 16; i++) mat_array.push_back(mat_ptr[i]);

    write_impl(key, mat_array);
}

void JsonSerializer::write_float(const std::string &key, float f) { write_impl(key, f); }

void JsonSerializer::write_str(const std::string &key, std::string str) { write_impl(key, str); }

void JsonSerializer::write_shape_kind(const std::string &key, ShapeKind shape_kind) {
    write_str(key, shape_kind_to_str[shape_kind]);
}

std::shared_ptr<AbstractSerializer<nlohmann::json>> JsonSerializer::new_object() {
    return std::make_shared<JsonSerializer>(nlohmann::json::object());
}
void JsonSerializer::write_array(
    const std::string &key,
    const std::vector<std::shared_ptr<AbstractSerializer<nlohmann::json>>> data_vector_serializer) {

    auto json_array = nlohmann::json::array();

    for (const auto &data_serializer: data_vector_serializer)
        json_array.push_back(data_serializer->get_data());

    write_impl(key, json_array);
}

void JsonSerializer::write_object(
    const std::string &key,
    const std::shared_ptr<AbstractSerializer<nlohmann::json>> data_serializer) {
    write_impl(key, data_serializer->get_data());
}

template<typename T>
void JsonSerializer::write_impl(const std::string &key, T value) {
    content[key] = value;
}

void JsonSerializer::to_file(std::filesystem::path output_file) {
    std::ofstream os(output_file, std::ios::out);
    os << content.dump();
    os.close();
}

nlohmann::json JsonSerializer::get_data() { return content; }

/*
 * Deserializer
 */

JsonDeserializer::JsonDeserializer(nlohmann::json object_json)
    : content(std::move(object_json)),
      shape_str_to_kind(
          {{"cylinder", CYLINDER}, {"sphere", SPHERE}, {"cube", CUBE}, {"feet", FEET}}) {}

JsonDeserializer::JsonDeserializer(const std::filesystem::path &object_json_path)
    : JsonDeserializer(nlohmann::json::parse(std::ifstream(object_json_path))) {}

glm::vec3 JsonDeserializer::read_vec3(const std::string &key) {
    auto vec3_json = read_object(key);
    return {vec3_json->read_float("x"), vec3_json->read_float("y"), vec3_json->read_float("z")};
}

glm::quat JsonDeserializer::read_quat(const std::string &key) {
    auto quat_json = read_object(key);
    return {
        quat_json->read_float("w"), quat_json->read_float("x"), quat_json->read_float("y"),
        quat_json->read_float("z")};
}

bool JsonDeserializer::read_bool(const std::string &key) { return read_impl<bool>(key); }

glm::mat4 JsonDeserializer::read_mat4(const std::string &key) {
    return glm::make_mat4(read_impl<std::vector<float>>(key).data());
}

float JsonDeserializer::read_float(const std::string &key) { return read_impl<float>(key); }

std::string JsonDeserializer::read_str(const std::string &key) {
    return read_impl<std::string>(key);
}

ShapeKind JsonDeserializer::read_shape_kind(const std::string &key) {
    return shape_str_to_kind[read_str(key)];
}

std::shared_ptr<AbstractDeserializer> JsonDeserializer::read_object(const std::string &key) {
    return std::make_shared<JsonDeserializer>(read_impl<nlohmann::json>(key));
}

std::vector<std::shared_ptr<AbstractDeserializer>>
JsonDeserializer::read_array(const std::string &key) {
    return transform_vector<nlohmann::json, std::shared_ptr<AbstractDeserializer>>(
        read_impl<nlohmann::json::array_t>(key),
        [](auto t) { return std::make_shared<JsonDeserializer>(t); });
}

template<typename T>
T JsonDeserializer::read_impl(const std::string &key) {
    return content[key].get<T>();
}
