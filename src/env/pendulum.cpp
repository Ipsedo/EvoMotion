//
// Created by samuel on 26/12/23.
//


#include "./pendulum.h"

Pendulum::Pendulum(int seed) : Environment({3}, {1}, true) {

}

std::vector<Item> Pendulum::get_items() {
    return std::vector<Item>();
}

std::vector<std::shared_ptr<Controller>> Pendulum::get_controllers() {
    return std::vector<std::shared_ptr<Controller>>();
}

step Pendulum::compute_step() {
    return step();
}

void Pendulum::reset_engine() {

}
