//
// Created by samuel on 26/12/23.
//

#ifndef EVO_MOTION_PENDULUM_H
#define EVO_MOTION_PENDULUM_H

#include "../model/environment.h"

class Pendulum : public Environment {
public:
    Pendulum(int seed);

    std::vector<Item> get_items() override;

    std::vector<std::shared_ptr<Controller>> get_controllers() override;

protected:
    step compute_step() override;

    void reset_engine() override;


};

#endif //EVO_MOTION_PENDULUM_H
