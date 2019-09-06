//
// Created by samuel on 13/08/19.
//

#include "test.h"
#include <random>

TestEnv::TestEnv(int seed) : Environment(renderer(1920, 1080), init_test(seed)) {

}

std::vector<item> TestEnv::init_test(int seed) {
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

	return items;
}

torch::IntArrayRef TestEnv::action_space() {
	return torch::IntArrayRef({1});
}

torch::IntArrayRef TestEnv::state_space() {
	torch::IntArrayRef({1});
}

TestEnv::~TestEnv() {

}

void TestEnv::act(torch::Tensor action) {
	return;
}

env_step TestEnv::compute_new_state() {
	return env_step();
}

env_step TestEnv::reset_engine() {
	return env_step();
}

