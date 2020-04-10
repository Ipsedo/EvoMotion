#include <CLI/CLI.hpp>

#include "tests/bullet_test.h"
#include "tests/opengl_test.h"
#include "tests/rl_test.h"
#include "utils/res.h"
#include "env/env_enum.h"
#include "rl/agent_enum.h"

std::string exec_root;

int main(int argc, char *argv[]) {
    exec_root = init_exec_root(argv[0]);

    CLI::App app{"EvoMotion"};

    app.require_subcommand(1);

    auto bullet_sc = app.add_subcommand("bullet", "Bullet test mode");
    auto opengl_sc = app.add_subcommand("opengl", "OpenGL test mode");
    auto rl_sc = app.add_subcommand("rl", "Reinforcement Learning mode");

    /*
     * RL parsing stuff
     */

    int nb_episode;
    std::string env, agent;
    int hidden_size;
    bool view;

    rl_sc->add_option<int>("-n,--nb-episode", nb_episode)->set_default_val("1000");
    rl_sc->add_option<std::string>("-e,--env", env)
            ->check(
            [](const std::string env_value) {
                auto env_choices = EnvEnum::get_names();

                if (std::find(env_choices.begin(), env_choices.end(), env_value) != env_choices.end())
                    return std::string("");

                std::string msg("RL environment must be = {");
                for (const std::string &e : env_choices)
                    msg.append(e + ", ");
                return msg.substr(0, msg.length() - 2).append("}. current = ").append(env_value);
            })->required(true);

    rl_sc->add_option<std::string>("-a,--agent", agent)
            ->check(
            [](const std::string agent_value) {
                auto agent_choices = AgentEnum::get_names();

                if (std::find(agent_choices.begin(), agent_choices.end(), agent_value) != agent_choices.end())
                    return std::string("");

                std::string msg("RL agent must be = {");
                for (const std::string &a : agent_choices)
                    msg.append(a + ", ");
                return msg.substr(0, msg.length() - 2).append("}. current = ").append(agent_value);
            })->required(true);

    rl_sc->add_flag("-v,--view", view, "Enable OpenGL view");

    rl_sc->require_subcommand(1);

    /*
     * Train rl parsing stuff
     */

    auto train_rl = rl_sc->add_subcommand("train", "Reinforcement Learning (train) mode");

    float epsilon, epsilon_decay, epsilon_min;
    int max_episode_step, max_cons_success;
    std::string output_folder;

    train_rl->add_option<float>("--epsilon", epsilon, "Epsilon value for training")
            ->set_default_val("0.7");
    train_rl->add_option<float>("--epsilon-decay", epsilon_decay, "Epsilon decay")
            ->set_default_val("0.9999");
    train_rl->add_option<float>("--epsilon-min", epsilon_min, "Minimum epsilon value")
            ->set_default_val("5e-3");
    train_rl->add_option<int>("--max-episode-step", max_episode_step, "Max episode step number")
            ->set_default_val("1000");
    train_rl->add_option<int>("--max-cons-success", max_cons_success, "Max consecutive success")
            ->set_default_val("10");
    train_rl->add_option("-o,--output-folder", output_folder, "Agent output folder")->required(true);
    train_rl->add_option<int>("--hidden-size", hidden_size)->set_default_val("24");

    /*
     * Test rl parsing stuff
     */

    auto test_rl = rl_sc->add_subcommand("test", "Reinforcement Learning (train) mode");

    std::string input_folder;

    test_rl->add_option("-i,--input-folder", input_folder, "Agent input folder")->required(true);

    /*
     * Parse args
     */

    CLI11_PARSE(app, argc, argv);

    if (bullet_sc->parsed()) test_bullet();
    else if (opengl_sc->parsed()) test_opengl();
    else if (rl_sc->parsed()) {

        if (train_rl->parsed()) {
            rl_train_info train_info;

            train_info.env_name = env;
            train_info.agent_name = agent;
            train_info.nb_episode = nb_episode;
            train_info.view = view == 1;
            train_info.hidden_size = hidden_size;
            train_info.max_episode_step = max_episode_step;
            train_info.max_consecutive_success = max_cons_success;
            train_info.eps = epsilon;
            train_info.eps_decay = epsilon_decay;
            train_info.eps_min = epsilon_min;
            train_info.out_model_folder = output_folder;

            train_reinforcement_learning(train_info);

        } else if (test_rl->parsed()) {
            rl_test_info test_info;

            test_info.env_name = env;
            test_info.agent_name = agent;
            test_info.nb_episode = nb_episode;
            test_info.view = view == 1;
            test_info.in_model_folder = input_folder;

            test_reinforcement_learning(test_info);
        }

    }

    return EXIT_SUCCESS;
}