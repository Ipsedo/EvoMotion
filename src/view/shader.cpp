/*****************************************************************************
From file: TD8-reflection-refraction-skel/utils.cpp
Informatique Graphique
Master d'informatique
Christian Jacquemin, Universite Paris-Sud & LIMSI-CNRS
Copyright (C) 2014 University Paris-Sud
This file is provided without support, instruction, or implied
warranty of any kind.  University Paris 11 makes no guarantee of its
fitness for a particular purpose and is not liable under any
circumstances for any damages or loss whatsoever arising from the use
or inability to use this file or items derived from it.
******************************************************************************/
#include <string>
#include "GL/glew.h"
#include <fstream>
#include <iostream>

unsigned long getFileLength(std::ifstream &file) {
    if (!file.good()) return 0;

    file.seekg(0, std::ios::end);
    unsigned long len = (unsigned long) file.tellg();
    file.seekg(std::ios::beg);

    return len;
}

GLuint load_shader(GLenum type, std::string filename) {

    GLuint shader = glCreateShader(type);
    GLchar *shaderSource;
    unsigned long len;

    std::ifstream file;
    file.open(filename.c_str(), std::ios::in); // opens as ASCII!
    if (!file) {
        printf("Error: shader file not found %s!\n", filename.c_str());
        throw 53;
    }

    len = getFileLength(file);
    if (len == 0) {
        printf("Error: empty shader file %s!\n", filename.c_str());
        throw 53;
    }

    shaderSource = new char[len + 1];

    shaderSource[len] = 0;

    unsigned int i = 0;
    while (file.good()) {
        shaderSource[i] = (GLchar) file.get();
        if (!file.eof())
            i++;
    }

    shaderSource[i] = 0;

    file.close();

    glShaderSource(shader, 1, (const char **) &shaderSource, NULL);

    delete[] shaderSource;

    return shader;
}

