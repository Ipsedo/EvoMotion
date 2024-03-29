//
// Created by samuel on 28/01/24.
//

#include "./env_test_muscle.h"

MuscleEnv::MuscleEnv(): Environment({1}, {1}, true), base("base", std::make_shared<ObjShape>("./resources/obj/cube.obj"), glm::vec3(0.f, -5.f, 5.f), glm::vec3(10.f, 1.f, 10.f), 0.f),
                        muscle("test_muscle", glm::vec3(0.1f), 0.01f, member_base, ) {

}

std::vector<Item> MuscleEnv::get_items() {
    return {base};
}

std::vector<std::shared_ptr<Controller>> MuscleEnv::get_controllers() {
    return {};
}

step MuscleEnv::compute_step() {
    return {
        torch::zeros({1}), 1, false
    };
}

void MuscleEnv::reset_engine() {

}
