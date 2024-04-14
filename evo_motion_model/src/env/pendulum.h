//
// Created by samuel on 26/12/23.
//

#ifndef EVO_MOTION_PENDULUM_H
#define EVO_MOTION_PENDULUM_H

#include <evo_motion_model/environment.h>

class Pendulum : public Environment {
public:
    Pendulum(int seed);

    std::vector<Item> get_items() override;

    std::vector<std::shared_ptr<Controller>> get_controllers() override;

    std::vector<int64_t> get_state_space() override;

    std::vector<int64_t> get_action_space() override;

    bool is_continuous() const override;

protected:
    step compute_step() override;

    void reset_engine() override;
};

#endif//EVO_MOTION_PENDULUM_H
