//
// Created by samuel on 15/12/22.
//

#include "./shader.h"

#include <filesystem>
#include <fstream>

#include "./constants.h"

unsigned long get_file_length(std::ifstream &file) {
    if (!file.good()) return 0;

    file.seekg(0, std::ios::end);
    const unsigned long len = file.tellg();
    file.seekg(std::ios::beg);

    return len;
}

uint load_shader(GLenum type, const std::string &filename) {
    int shader = glCreateShader(type);
    GLchar *shaderSource;
    unsigned long len;

    std::filesystem::path shader_source_path(SHADERS_SOURCE_PATH);
    std::ifstream file(shader_source_path / filename, std::ios::in);

    len = get_file_length(file);

    shaderSource = new char[len + 1];

    shaderSource[len] = 0;

    unsigned int i = 0;
    while (file.good()) {
        shaderSource[i] = static_cast<GLchar>(file.get());
        if (!file.eof()) i++;
    }

    shaderSource[i] = 0;

    file.close();

    glShaderSource(shader, 1, const_cast<const char **>(&shaderSource), nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) throw std::runtime_error("Shader compile failed");

    delete[] shaderSource;

    return shader;
}