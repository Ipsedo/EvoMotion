//
// Created by samuel on 20/01/25.
//

#ifndef EVO_MOTION_JSON_SERIALIZER_H
#define EVO_MOTION_JSON_SERIALIZER_H

#include <nlohmann/json.hpp>

#include <evo_motion_model/serializer.h>

/*
 * Serializer
 */

class JsonSerializer final : public AbstractSerializer {
public:
    explicit JsonSerializer();
    explicit JsonSerializer(nlohmann::json content);

    void write_vec3(const std::string &key, glm::vec3 vec) override;
    void write_quat(const std::string &key, glm::quat quat) override;
    void write_bool(const std::string &key, bool value) override;
    void write_mat4(const std::string &key, glm::mat4 mat) override;
    void write_float(const std::string &key, float f) override;
    void write_str(const std::string &key, std::string str) override;
    void write_shape_kind(const std::string &key, ShapeKind shape_kind) override;

    std::shared_ptr<AbstractSerializer> new_object() override;
    void write_array(
        const std::string &key,
        std::vector<std::shared_ptr<AbstractSerializer>> data_vector_serializer) override;
    void write_object(
        const std::string &key, std::shared_ptr<AbstractSerializer> data_serializer) override;

    void to_file(std::filesystem::path output_file) override;

    std::any get_data() override;

private:
    nlohmann::json content;
    std::map<ShapeKind, std::string> shape_kind_to_str;

    template<typename T>
    void write_impl(const std::string &key, T value);
};

/*
 * Deserializer
 */

class JsonDeserializer final : public AbstractDeserializer {
public:
    explicit JsonDeserializer(nlohmann::json object_json);
    explicit JsonDeserializer(const std::filesystem::path &object_json_path);

    glm::vec3 read_vec3(const std::string &key) override;
    glm::quat read_quat(const std::string &key) override;
    bool read_bool(const std::string &key) override;
    glm::mat4 read_mat4(const std::string &key) override;
    float read_float(const std::string &key) override;
    std::string read_str(const std::string &key) override;
    ShapeKind read_shape_kind(const std::string &key) override;

    std::shared_ptr<AbstractDeserializer> read_object(const std::string &key) override;
    std::vector<std::shared_ptr<AbstractDeserializer>> read_array(const std::string &key) override;

private:
    nlohmann::json content;
    std::map<std::string, ShapeKind> shape_str_to_kind;

    template<typename T>
    T read_impl(const std::string &key);
};

#endif//EVO_MOTION_JSON_SERIALIZER_H
