//
// Created by samuel on 25/12/22.
//

#include "slider.h"

SliderController::SliderController(int action_index, btSliderConstraint *slider, float slider_speed)
    : action_index(action_index), slider(slider), slider_speed(slider_speed) {}

void SliderController::on_input(torch::Tensor action) {
    slider->setTargetLinMotorVelocity(action[action_index].item().toFloat() * slider_speed);
}
