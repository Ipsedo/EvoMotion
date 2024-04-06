//
// Created by samuel on 26/12/23.
//


#include "./pendulum.h"

Pendulum::Pendulum(int seed) : Environment() {

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

std::vector<int64_t> Pendulum::get_state_space() {
    return {3};
}

std::vector<int64_t> Pendulum::get_action_space() {
    return {1};
}

bool Pendulum::is_continuous() const {
    return true;
}
