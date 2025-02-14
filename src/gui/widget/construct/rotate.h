//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_ROTATE_H
#define EVO_MOTION_ROTATE_H

#include <functional>

class RotateWidget {
public:
    explicit RotateWidget(const std::function<void(float, float, float)> &on_rotate);

    void render_widget();

private:
    std::function<void(float, float, float)> on_rotate;
};

#endif//EVO_MOTION_ROTATE_H
