//
// Created by samuel on 15/12/22.
//


#include <filesystem>
#include <fstream>
#include <iostream>

#include <shader.h>

unsigned long get_file_length(std::ifstream &file) {
    if (!file.good()) return 0;

    file.seekg(0, std::ios::end);
    unsigned long len = (unsigned long) file.tellg();
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
        shaderSource[i] = (GLchar) file.get();
        if (!file.eof()) i++;
    }

    shaderSource[i] = 0;

    file.close();

    glShaderSource(shader, 1, (const char **) &shaderSource, nullptr);
    glCompileShader(shader);

    delete[] shaderSource;

    return shader;
}
