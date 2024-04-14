//
// Created by samuel on 31/03/24.
//

#include "./muscle_controller.h"

#include <utility>

MuscleController::MuscleController(Muscle muscle, int action_index)
    : action_index(action_index), muscle(std::move(muscle)) {}

void MuscleController::on_input(torch::Tensor action) {
    muscle.contract(action[action_index].item().toFloat());
}
