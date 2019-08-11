//
// Created by samuel on 11/08/19.
//

#include "env_list.h"
#include "../model/body.h"

environment create_test_env() {
    int nb_box = 10;

    float max_height = 10.f, min_height = 5.f, max_scale = 0.1f;

    auto rend = renderer(1920, 1080);
    rend.init();

    std::vector<item> items;
    for (int i = 0; i < nb_box; i++) {
        glm::vec3 pos(0.f, (float(rand()) / RAND_MAX) * (max_height - min_height) + max_height, 10.f);
        items.push_back(create_item_box(pos, glm::mat4(1.f), glm::vec3(max_scale * float(rand()) / RAND_MAX), 10.f));
    }

    items.push_back(create_item_box(glm::vec3(0.f, -3.f, 10.f), glm::mat4(1.f), glm::vec3(1.f), 0.f));

    return environment(rend, items, true);
}
