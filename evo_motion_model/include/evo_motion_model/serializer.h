//
// Created by samuel on 19/01/25.
//

#ifndef EVO_MOTION_SERIALIZER_H
#define EVO_MOTION_SERIALIZER_H

#include <any>
#include <filesystem>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "./shapes.h"

class AbstractSerializer {
public:
    virtual ~AbstractSerializer() = default;

    virtual void write_vec3(const std::string &key, glm::vec3 vec) = 0;
    virtual void write_quat(const std::string &key, glm::quat quat) = 0;
    virtual void write_bool(const std::string &key, bool value) = 0;
    virtual void write_mat4(const std::string &key, glm::mat4 mat) = 0;
    virtual void write_float(const std::string &key, float f) = 0;
    virtual void write_str(const std::string &key, std::string str) = 0;
    virtual void write_shape_kind(const std::string &key, ShapeKind shape_kind) = 0;

    virtual std::shared_ptr<AbstractSerializer> new_object() = 0;

    virtual void write_array(
        const std::string &key,
        std::vector<std::shared_ptr<AbstractSerializer>> data_vector_serializer) = 0;
    virtual void
    write_object(const std::string &key, std::shared_ptr<AbstractSerializer> data_serializer) = 0;

    virtual void to_file(std::filesystem::path output_file) = 0;

    virtual std::any get_data() = 0;
};

class AbstractDeserializer {
public:
    virtual ~AbstractDeserializer() = default;

    virtual glm::vec3 read_vec3(const std::string &key) = 0;
    virtual glm::quat read_quat(const std::string &key) = 0;
    virtual bool read_bool(const std::string &key) = 0;
    virtual glm::mat4 read_mat4(const std::string &key) = 0;
    virtual float read_float(const std::string &key) = 0;
    virtual std::string read_str(const std::string &key) = 0;
    virtual ShapeKind read_shape_kind(const std::string &key) = 0;

    virtual std::shared_ptr<AbstractDeserializer> read_object(const std::string &key) = 0;
    virtual std::vector<std::shared_ptr<AbstractDeserializer>>
    read_array(const std::string &key) = 0;
};

#endif//EVO_MOTION_SERIALIZER_H
