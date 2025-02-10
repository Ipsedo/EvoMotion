//
// Created by samuel on 24/01/25.
//

#ifndef EVO_MOTION__CONVERT_H
#define EVO_MOTION__CONVERT_H

#include <bitset>
#include <fstream>
#include <optional>
#include <queue>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

glm::mat4 to_mat4(nlohmann::json json_transformation);
float round_found(float f, int precision);
std::string float_to_binary_string(float f);
float binary_string_to_float(const std::string &s);
void convert_tree_skeleton_to_graph_skeleton();

#endif//EVO_MOTION__CONVERT_H
