//
// Created by samuel on 13/02/25.
//

#include "./utils.h"

#include <string>

#include <imgui.h>

bool input_float(
    const char *name, float *value, const int precision, const ImGuiInputTextFlags flags) {
    const ImVec2 name_size = ImGui::CalcTextSize(name);
    const ImVec2 float_size = ImGui::CalcTextSize(std::string(precision + 3, '0').c_str());

    const float width = name_size.x + float_size.x + ImGui::GetStyle().FramePadding.x * 4.f;
    std::string format = "%.";
    format += std::to_string(precision) + "f";

    ImGui::SetNextItemWidth(width);
    return ImGui::InputFloat(name, value, 0.f, 0.f, format.c_str(), flags);
}

bool input_text(
    const char *name, char *buf, const size_t buf_size, const int nb_char,
    const ImGuiInputTextFlags flags) {
    const ImVec2 name_size = ImGui::CalcTextSize(name);
    const ImVec2 default_input_size = ImGui::CalcTextSize(std::string(nb_char, 'a').c_str());

    float width = name_size.x + default_input_size.x + ImGui::GetStyle().FramePadding.x * 4.f;

    ImGui::SetNextItemWidth(width);
    return ImGui::InputText(name, buf, buf_size, flags);
}
