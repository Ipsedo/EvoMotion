//
// Created by samuel on 15/12/22.
//

#ifndef EVO_MOTION_SHAPES_H
#define EVO_MOTION_SHAPES_H

#include <string>
#include <tuple>
#include <vector>

enum ShapeKind { CUBE, SPHERE, CYLINDER, FEET };

class Shape {
public:
    virtual std::vector<std::tuple<float, float, float>> get_vertices() = 0;

    virtual std::vector<std::tuple<float, float, float>> get_normals() = 0;

    virtual ~Shape();
};

class ObjShape final : public Shape {
    std::vector<std::tuple<float, float, float>> vertices;
    std::vector<std::tuple<float, float, float>> normals;

public:
    explicit ObjShape(const std::string &obj_file_path);

    std::vector<std::tuple<float, float, float>> get_vertices() override;

    std::vector<std::tuple<float, float, float>> get_normals() override;

    ~ObjShape() override;
};

#endif//EVO_MOTION_SHAPES_H