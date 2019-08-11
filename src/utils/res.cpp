//
// Created by samuel on 11/08/19.
//

#include "res.h"

#include <vector>

#include "res.h"
#include "string_utils.h"

std::string get_res_folder() {
    std::string this_file = __FILE__;
    std::vector<std::string> splited_path = split(this_file, EVOMOTION_SEP);

    while (splited_path.back() != "EvoMotion") {
        splited_path.pop_back();
    }

    std::string res_path;
    for (auto &elt : splited_path) {
        res_path += EVOMOTION_SEP + elt;
    }

    res_path += EVOMOTION_SEP + std::string("res");

    return res_path.substr(1, res_path.size());
}

std::string get_shader_folder() {
    std::string this_file = __FILE__;
    std::vector<std::string> splited_path = split(this_file, EVOMOTION_SEP);

    while (splited_path.back() != "EvoMotion") {
        splited_path.pop_back();
    }

    std::string res_path;
    for (auto &elt : splited_path) {
        res_path += EVOMOTION_SEP + elt;
    }

    res_path += EVOMOTION_SEP + std::string("shaders");

    return res_path.substr(1, res_path.size());
}