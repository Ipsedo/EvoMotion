//
// Created by samuel on 25/12/22.
//

#include "slider.h"

SliderController::SliderController(
    const int action_index, btSliderConstraint *slider, const float slider_speed)
    : action_index(action_index), slider(slider), slider_speed(slider_speed) {}

void SliderController::on_input(const torch::Tensor action) {
    slider->setTargetLinMotorVelocity(action[action_index].item().toFloat() * slider_speed);
}