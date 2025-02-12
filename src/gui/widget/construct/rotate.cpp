//
// Created by samuel on 12/02/25.
//

#include "./rotate.h"

RotateWidget::RotateWidget(const std::function<void(float, float, float)> &on_rotate)
    : on_rotate(on_rotate) {}
void RotateWidget::render_widget() {}
