//
// Created by samuel on 11/08/19.
//

#include "string_utils.h"

#include <sstream>

std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        //elems.push_back(item);
        elems.push_back(move(item)); // if C++11 (based on comment from @mchiasson)
    }
    return elems;
}