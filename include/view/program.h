//
// Created by samuel on 15/12/22.
//

#ifndef EVO_MOTION_PROGRAM_H
#define EVO_MOTION_PROGRAM_H

#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#include <glm/glm.hpp>


class Program {
public:
    class Builder {
    public:
        Builder(std::string vertex_shader_path, std::string fragment_shader_path);

        Program::Builder add_uniform(const std::string &name);

        Program::Builder add_attribute(const std::string &name);

        Program::Builder add_buffer(const std::string &name, const std::vector<float> &data);

        Program build();

    private:
        Builder(
                std::string vertex_shader_path,
                std::string fragment_shader_path,

                const std::vector<std::string> &uniforms,
                const std::vector<std::string> &attributes,

                const std::map<std::string, std::vector<float>> &buffers
                );

        std::string vertex_shader_path;
        std::string fragment_shader_path;

        std::vector<std::string> uniforms;
        std::vector<std::string> attributes;

        std::map<std::string, std::vector<float>> buffers;
    };

private:

    GLuint program_id;
    GLuint vertex_shader_id;
    GLuint fragment_shader_id;

    std::map<std::string, GLuint> uniform_handles;
    std::map<std::string, GLuint> attribute_handles;

    std::map<std::string, GLuint> buffer_ids;

    template<typename F, class... T>
    void _uniform(F uniform_fun, const std::string &name, T... args);

protected:
public:
    void use() const;

    void uniform_mat4(const std::string &name, glm::mat4 mat4);

    void uniform_vec4(const std::string &name, glm::vec4 vec4);

    void uniform_vec3(const std::string &name, glm::vec3 vec3);

    void uniform_float(const std::string &name, float f);

    void attrib(const std::string &name, const std::string &buffer_name, int data_size, int stride, int offset);

    void disable_attrib_array();

    void kill();
};

#endif //EVO_MOTION_PROGRAM_H
