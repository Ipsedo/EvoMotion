//
// Created by samuel on 15/01/25.
//

#include <evo_motion_model/json/json_muscle.h>

#include "../converter.h"

/*
 * JSON
 */

JsonMuscularSystem::JsonMuscularSystem(Skeleton skeleton, const std::string &json_path) {
    auto json_muscles = read_json(json_path)["muscles"];

    for (auto json_muscle: json_muscles) {
        Item item_a = skeleton.get_item(
            skeleton.get_robot_name() + "_" + json_muscle["item_a"].get<std::string>());
        Item item_b = skeleton.get_item(
            skeleton.get_robot_name() + "_" + json_muscle["item_b"].get<std::string>());

        muscles.emplace_back(
            skeleton.get_robot_name().append("_").append(json_muscle["name"].get<std::string>()),
            json_muscle["attach_mass"].get<float>(),
            json_vec3_to_glm_vec3(json_muscle["attach_scale"]), item_a,
            json_vec3_to_glm_vec3(json_muscle["pos_in_a"]), item_b,
            json_vec3_to_glm_vec3(json_muscle["pos_in_b"]), json_muscle["force"].get<float>(),
            json_muscle["speed"].get<float>());
    }
}

std::vector<Muscle> JsonMuscularSystem::get_muscles() { return muscles; }