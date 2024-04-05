//
// Created by samuel on 28/01/24.
//

#ifndef EVO_MOTION_ENV_TEST_MUSCLE_H
#define EVO_MOTION_ENV_TEST_MUSCLE_H

#include "../model/environment.h"
#include "../model/muscle.h"

class MuscleEnv : public Environment {
public:
    MuscleEnv();

    std::vector<Item> get_items() override;

    std::vector<std::shared_ptr<Controller>> get_controllers() override;

    std::vector<int64_t> get_state_space() override;

    std::vector<int64_t> get_action_space() override;

    bool is_continuous() const override;

protected:
    step compute_step() override;

    void reset_engine() override;

private:
    std::vector<Item> items;
    std::vector<btTypedConstraint *> constraints;

    std::vector<std::shared_ptr<Controller>> controllers;

};

#endif //EVO_MOTION_ENV_TEST_MUSCLE_H
