//
// Created by samuel on 17/12/22.
//

#ifndef EVO_MOTION_SPECULAR_H
#define EVO_MOTION_SPECULAR_H

#include <filesystem>
#include <functional>
#include <optional>
#include <vector>

#include <glm/glm.hpp>

#include <evo_motion_view/drawable.h>

#include "../program.h"

class OBjSpecular : public Drawable {
protected:
    glm::vec4 ambient_color;
    glm::vec4 diffuse_color;
    glm::vec4 specular_color;

private:
    const int position_size;
    const int normal_size;
    const int stride;

    float shininess;

    int nb_vertices;

    Program program;

    static std::vector<float> to_vbo_data(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals);

public:
    OBjSpecular(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color,
        glm::vec4 diffuse_color, glm::vec4 specular_color, float shininess);

    void draw(
        glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix,
        glm::vec3 light_pos_from_camera, glm::vec3 camera_pos) override;

    ~OBjSpecular() override;
};

/*
 * MTL Obj
 */

class ObjMtlSpecular : public Drawable {
public:
    ObjMtlSpecular(
        const std::filesystem::path &obj_file_path, const std::filesystem::path &mtl_file_path,
        float ambient_color_factor, float diffuse_color_factor, float specular_color_factor);
    void draw(
        glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix,
        glm::vec3 light_pos_from_camera, glm::vec3 camera_pos) override;

private:
    const int position_size;
    const int normal_size;
    const int color_size;
    const int shininess_size;

    const int stride;

    int nb_vertices;

    Program program;

    float ambient_color_factor;
    float diffuse_color_factor;
    float specular_color_factor;

    static std::vector<std::string> split(const std::string &s, const char delim);

    std::vector<float> read_packed_data(
        const std::filesystem::path &obj_file_path, const std::filesystem::path &mtl_file_path);
};

/*
 * Builder
 */

class BuilderObjSpecular final : public OBjSpecular {
public:
    BuilderObjSpecular(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals, const glm::vec4 &ambient_color,
        const glm::vec4 &diffuse_color, const glm::vec4 &specular_color, float shininess,
        const std::optional<std::function<std::optional<glm::vec3>()>> &is_focus_function,
        const std::optional<std::function<bool()>> &is_hidden_function);
    void draw(
        glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix,
        glm::vec3 light_pos_from_camera, glm::vec3 camera_pos) override;

private:
    std::function<std::optional<glm::vec3>()> is_focus_function;
    std::function<bool()> is_hidden_function;
};

#endif//EVO_MOTION_SPECULAR_H