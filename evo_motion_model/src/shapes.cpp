//
// Created by samuel on 15/12/22.
//

#include <fstream>
#include <iostream>
#include <sstream>

#include <evo_motion_model/shapes.h>

std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) { elems.push_back(item); }
    return elems;
}

ObjShape::ObjShape(const std::string &obj_file_path) {
    std::vector<std::tuple<float, float, float>> vertices_ref;
    std::vector<std::tuple<float, float, float>> normals_ref;

    std::vector<int> vertices_order;
    std::vector<int> normals_order;

    std::ifstream obj_file(obj_file_path);
    std::string line;

    while (std::getline(obj_file, line)) {
        std::vector<std::string> split_line = split(line, ' ');

        if (split_line[0] == "vn") {
            normals_ref.emplace_back(
                std::stof(split_line[1]), std::stof(split_line[2]), std::stof(split_line[3]));
        } else if (split_line[0] == "v") {
            vertices_ref.emplace_back(
                std::stof(split_line[1]), std::stof(split_line[2]), std::stof(split_line[3]));

        } else if (split_line[0] == "f") {
            vertices_order.push_back(std::stoi(split(split_line[1], '/')[0]));
            vertices_order.push_back(std::stoi(split(split_line[2], '/')[0]));
            vertices_order.push_back(std::stoi(split(split_line[3], '/')[0]));

            normals_order.push_back(std::stoi(split(split_line[1], '/')[2]));
            normals_order.push_back(std::stoi(split(split_line[2], '/')[2]));
            normals_order.push_back(std::stoi(split(split_line[3], '/')[2]));
        }
    }

    for (int i = 0; i < vertices_order.size(); i++) {
        vertices.push_back(vertices_ref[vertices_order[i] - 1]);
        normals.push_back(normals_ref[normals_order[i] - 1]);
    }
}

std::vector<std::tuple<float, float, float>> ObjShape::get_vertices() { return vertices; }

std::vector<std::tuple<float, float, float>> ObjShape::get_normals() { return normals; }
