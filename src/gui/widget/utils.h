//
// Created by samuel on 13/02/25.
//

#ifndef EVO_MOTION_UTILS_H
#define EVO_MOTION_UTILS_H

#include <string>

#include <imgui.h>

bool input_float(const char *name, float *value, int precision, ImGuiInputTextFlags flags = 0);

bool input_text(
    const char *name, char *buf, size_t buf_size, int nb_char, ImGuiInputTextFlags flags = 0);

#endif//EVO_MOTION_UTILS_H
