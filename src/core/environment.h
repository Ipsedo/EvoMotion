//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_ENVIRONMENT_H
#define EVOMOTION_ENVIRONMENT_H

#include <btBulletDynamicsCommon.h>
#include <torch/torch.h>
#include "../view/obj_mtl_vbo.h"
#include "../view/renderer.h"
#include "../model/engine.h"

struct env_step {
	torch::Tensor state;
	float reward;
	bool done;
};

class Environment {
private:
	int m_step;

protected:
	// Model
	engine m_engine;

	// View
	renderer m_renderer;

	// Items
	std::vector<item> m_items;

	/**
	 * Virtual method applying action
	 * Need to be overloaded !
	 * @param action
	 */
	virtual void act(torch::Tensor action) = 0;

	/**
	 * Virtual method to compute next state
	 * @return the new computed state, reward and done infos
	 */
	virtual env_step compute_new_state() = 0;

	/**
	 * Virtual method to reset bullet physical engine
	 * @return the intial state, reward and done infos
	 */
	virtual env_step reset_engine() = 0;

public:
	Environment(renderer renderer, std::vector<item> items);

	/**
	 * Get the action shape
	 * @return a torch::IntArrayRef representing the action shape
	 */
	virtual torch::IntArrayRef action_space() = 0;

	/**
	 * Get the state shape
	 * @return a torch::IntArrayRef representing the state shape
	 */
	virtual torch::IntArrayRef state_space() = 0;

	/**
	 * Reset the environment
	 * @return the initial state, reward and done infos
	 */
	const env_step reset();

	/**
	 * Step the environment
	 * @param delta the time delta in seconds
	 * @param action the action to perform
	 * @param will_draw indicate if drawing environment is needed
	 * @return the next state, reward and done infos
	 */
	const env_step step(float delta, torch::Tensor action, bool will_draw);

	bool is_renderer_on();

	virtual bool is_action_discrete() = 0;

	virtual ~Environment();

};

#endif //EVOMOTION_ENVIRONMENT_H
