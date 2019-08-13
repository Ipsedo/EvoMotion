//
// Created by samuel on 13/08/19.
//

#include "test.h"

environment create_test_env() {
    int seed = 12345;

    std::default_random_engine rd_gen(seed);
    std::uniform_real_distribution<float> rd_uni(0.f, 1.f);

    int nb_box = 10;

    float max_height = 10.f, min_height = 5.f, max_scale = 0.1f;

    auto rend = renderer(1920, 1080);

    std::vector<item> items;
    for (int i = 0; i < nb_box; i++) {
        glm::vec3 pos(0.f, rd_uni(rd_gen) * (max_height - min_height) + max_height, 10.f);
        items.push_back(create_item_box(pos, glm::mat4(1.f), glm::vec3(max_scale * rd_uni(rd_gen)), 10.f));
    }

    items.push_back(create_item_box(glm::vec3(0.f, -3.f, 10.f), glm::mat4(1.f), glm::vec3(1.f), 0.f));

    auto act = [](torch::Tensor action, std::vector<item> items) {
        return;
    };

    auto step = [](std::vector<item> items) {
        return env_step();
    };

    return environment(rend, items,
            torch::IntArrayRef({1}), torch::IntArrayRef({1}),
            act, step, [](std::vector<item> v){return;});
}