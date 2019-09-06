//
// Created by samuel on 13/08/19.
//

#ifndef EVOMOTION_TEST_H
#define EVOMOTION_TEST_H

#include "../environment.h"

class TestEnv : public Environment {
public:
	torch::IntArrayRef action_space() override;

	torch::IntArrayRef state_space() override;

	~TestEnv() override;

	TestEnv(int seed);

protected:
	void act(torch::Tensor action) override;

	env_step compute_new_state() override;

	env_step reset_engine() override;

private:
	std::vector<item> init_test(int seed);

};

#endif //EVOMOTION_TEST_H
