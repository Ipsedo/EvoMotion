//
// Created by samuel on 31/03/24.
//

#include <utility>

#include "./muscle_controller.h"

MuscleController::MuscleController(Muscle muscle, int action_index)
    : action_index(action_index), muscle(std::move(muscle)) {}

void MuscleController::on_input(torch::Tensor action) {
    muscle.contract(action[action_index].item().toFloat());
}
