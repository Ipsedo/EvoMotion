//
// Created by samuel on 31/03/24.
//

#ifndef EVO_MOTION_MUSCLE_CONTROLLER_H
#define EVO_MOTION_MUSCLE_CONTROLLER_H

#include "../creature/muscle.h"
#include "evo_motion_model/controller.h"

class MuscleController : public Controller {
public:
    MuscleController(Muscle muscle, int action_index);

    void on_input(torch::Tensor action) override;

private:
    int action_index;
    Muscle muscle;
};

#endif//EVO_MOTION_MUSCLE_CONTROLLER_H
