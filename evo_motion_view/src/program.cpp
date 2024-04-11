//
// Created by samuel on 15/12/22.
//

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

#include <evo_motion_view/constants.h>
#include <evo_motion_view/program.h>

#include "./shader.h"

/*
 * Buffer builder
 */

Program::Builder::Builder(std::string vertex_shader_path, std::string fragment_shader_path)
    : vertex_shader_path(std::move(vertex_shader_path)),
      fragment_shader_path(std::move(fragment_shader_path)) {}

Program::Builder::Builder(
    std::string vertex_shader_path, std::string fragment_shader_path,
    const std::vector<std::string> &uniforms, const std::vector<std::string> &attributes,
    const std::map<std::string, std::vector<float>> &buffers)
    : vertex_shader_path(std::move(vertex_shader_path)),
      fragment_shader_path(std::move(fragment_shader_path)), uniforms(uniforms),
      attributes(attributes), buffers(buffers) {}

Program::Builder Program::Builder::add_uniform(const std::string &name) {
    uniforms.push_back(name);
    return {vertex_shader_path, fragment_shader_path, uniforms, attributes, buffers};
}

Program::Builder Program::Builder::add_attribute(const std::string &name) {
    attributes.push_back(name);
    return {vertex_shader_path, fragment_shader_path, uniforms, attributes, buffers};
}

Program::Builder
Program::Builder::add_buffer(const std::string &name, const std::vector<float> &data) {
    buffers.insert({name, data});
    return {vertex_shader_path, fragment_shader_path, uniforms, attributes, buffers};
}

Program Program::Builder::build() {
    Program program{};

    program.program_id = glCreateProgram();

    program.vertex_shader_id = load_shader(GL_VERTEX_SHADER, vertex_shader_path);
    program.fragment_shader_id = load_shader(GL_FRAGMENT_SHADER, fragment_shader_path);

    glAttachShader(program.program_id, program.vertex_shader_id);
    glAttachShader(program.program_id, program.fragment_shader_id);

    glLinkProgram(program.program_id);

    for (const auto &[name, data]: buffers) {
        program.buffer_ids.insert({name, 0});

        glGenBuffers(1, &program.buffer_ids[name]);

        glBindBuffer(GL_ARRAY_BUFFER, program.buffer_ids[name]);
        glBufferData(GL_ARRAY_BUFFER, int(data.size()) * BYTES_PER_FLOAT, &data[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    for (const auto &name: uniforms)
        program.uniform_handles.insert(
            {name, glGetUniformLocation(program.program_id, name.c_str())});

    for (const auto &name: attributes)
        program.attribute_handles.insert(
            {name, glGetAttribLocation(program.program_id, name.c_str())});

    return program;
}

/*
 * Program
 */

void Program::kill() {
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    for (auto [name, buffer_id]: buffer_ids) glDeleteBuffers(1, &buffer_id);

    glDeleteProgram(program_id);
}

void Program::use() const { glUseProgram(program_id); }

template<typename F, class... T>
void Program::_uniform(F uniform_fun, const std::string &name, T... args) {
    uniform_fun(uniform_handles[name], args...);
}

void Program::uniform_mat4(const std::string &name, glm::mat4 mat4) {
    _uniform(glUniformMatrix4fv, name, 1, GL_FALSE, glm::value_ptr(mat4));
}

void Program::uniform_vec4(const std::string &name, glm::vec4 vec4) {
    _uniform(glUniform4fv, name, 1, glm::value_ptr(vec4));
}

void Program::uniform_vec3(const std::string &name, glm::vec3 vec3) {
    _uniform(glUniform3fv, name, 1, glm::value_ptr(vec3));
}

void Program::uniform_float(const std::string &name, float f) { _uniform(glUniform1f, name, f); }

void Program::attrib(
    const std::string &name, const std::string &buffer_name, int data_size, int stride,
    int offset) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer_ids[buffer_name]);

    glEnableVertexAttribArray(attribute_handles[name]);
    glVertexAttribPointer(
        attribute_handles[name], data_size, GL_FLOAT, GL_FALSE, stride, (char *) nullptr + offset);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Program::disable_attrib_array() {
    for (auto [name, attrib_id]: attribute_handles) glDisableVertexAttribArray(attrib_id);
}

void Program::draw_arrays(GLenum type, int from, int nb_vertices) {
    glDrawArrays(type, from, nb_vertices);
}
