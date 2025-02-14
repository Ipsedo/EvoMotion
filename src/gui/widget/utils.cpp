//
// Created by samuel on 13/02/25.
//

#include "./utils.h"

#include <string>

#include <imgui.h>

bool input_float(
    const char *name, float *value, const int precision, const ImGuiInputTextFlags flags) {
    std::string format = "%.";
    format += std::to_string(precision) + "f";

    std::string input_float_name(name);
    input_float_name = "##" + input_float_name;

    bool set = ImGui::InputFloat(input_float_name.c_str(), value, 0.f, 0.f, format.c_str(), flags);
    ImGui::SameLine();
    ImGui::Text("%s", name);

    return set;
}
