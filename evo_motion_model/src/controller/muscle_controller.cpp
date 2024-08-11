//
// Created by samuel on 31/03/24.
//

#include "./muscle_controller.h"


MuscleController::MuscleController(const Muscle &muscle, const int action_index)
    : action_index(action_index), muscle(muscle) {}

void MuscleController::on_input(const torch::Tensor action) {
    muscle.contract(action[action_index].item().toFloat());
}

MuscleController::~MuscleController() = default;