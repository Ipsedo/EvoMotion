//
// Created by samuel on 31/03/24.
//

#ifndef EVO_MOTION_MUSCLE_CONTROLLER_H
#define EVO_MOTION_MUSCLE_CONTROLLER_H

#include <evo_motion_model/controller.h>
#include <evo_motion_model/robot/muscle.h>

class MuscleController final : public Controller {
public:
    MuscleController(const std::shared_ptr<Muscle> &muscle, int action_index);

    void on_input(torch::Tensor action) override;

    ~MuscleController() override;

private:
    int action_index;
    std::shared_ptr<Muscle> muscle;
};

#endif//EVO_MOTION_MUSCLE_CONTROLLER_H