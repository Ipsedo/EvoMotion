//
// Created by samuel on 18/12/22.
//

#ifndef EVO_MOTION_ENVIRONMENT_H
#define EVO_MOTION_ENVIRONMENT_H

#include <vector>

#include <btBulletDynamicsCommon.h>
#include <torch/torch.h>

#include "./controller.h"
#include "./item.h"

struct step {
    torch::Tensor state;
    float reward;
    bool done;
};

class Environment {
protected:
    torch::DeviceType curr_device;

    btDefaultCollisionConfiguration *m_collision_configuration;
    btCollisionDispatcher *m_dispatcher;
    btBroadphaseInterface *m_broad_phase;
    btSequentialImpulseConstraintSolver *m_constraint_solver;
    btDynamicsWorld *m_world;

    virtual step compute_step() = 0;

    virtual void reset_engine() = 0;

    void add_item(const Item &item) const;

public:
    Environment();

    virtual std::vector<Item> get_items() = 0;

    virtual std::vector<std::shared_ptr<Controller>> get_controllers() = 0;

    step do_step(const torch::Tensor &action);

    step reset();

    virtual std::vector<int64_t> get_state_space() = 0;

    virtual std::vector<int64_t> get_action_space() = 0;

    [[nodiscard]] virtual bool is_continuous() const = 0;

    void to(torch::DeviceType device);

    virtual ~Environment();
};

class EnvironmentFactory {
public:
    explicit EnvironmentFactory(std::map<std::string, std::string> parameters);
    virtual std::shared_ptr<Environment> get_env(int seed) = 0;

private:
    std::map<std::string, std::string> parameters;

protected:
    template<typename Value>
    Value get_value(const std::string &key, Value default_value);

    template<typename Value>
    Value generic_get_value(
        std::function<Value(const std::string &)> converter, const std::string &key,
        Value default_value);
};

std::shared_ptr<EnvironmentFactory>
get_environment_factory(const std::string &env_name, std::map<std::string, std::string> parameters);

#endif//EVO_MOTION_ENVIRONMENT_H