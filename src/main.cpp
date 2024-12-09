//
// Created by samuel on 15/12/22.
//

#include <iostream>
#include <memory>

#include <argparse/argparse.hpp>

#include <evo_motion_networks/agent.h>

#include "./run.h"

std::pair<std::string, std::string> parse_pair(const std::string &k_v) {
    const auto delimiter = '=';

    if (std::count(k_v.begin(), k_v.end(), delimiter) != 1) throw std::invalid_argument(k_v);

    const std::string key = k_v.substr(0, k_v.find(delimiter));
    const std::string value = k_v.substr(k_v.find(delimiter) + 1);

    return {key, value};
}

int main(const int argc, char **argv) {
    argparse::ArgumentParser parser("evo_motion");

    // GUI

    argparse::ArgumentParser gui_parser("gui");

    // CLI

    argparse::ArgumentParser cli_parser("cli");

    cli_parser.add_argument("environment").default_value("cartpole").help("the environment");
    cli_parser.add_argument("agent").default_value("actor_critic_liquid").help("the agent");

    cli_parser.add_argument("--agent_parameters")
        .nargs(argparse::nargs_pattern::any)
        .action(parse_pair)
        .help("agent hyper-parameters");

    cli_parser.add_argument("--env_parameters")
        .nargs(argparse::nargs_pattern::any)
        .action(parse_pair)
        .help("environment parameters");

    cli_parser.add_argument("--env_seed")
        .scan<'i', int>()
        .default_value(1234)
        .help("seed for environment RNG");

    cli_parser.add_argument("--env_num_threads")
        .scan<'i', int>()
        .default_value(8)
        .help("seed for environment RNG");

    cli_parser.add_argument("--cuda").default_value(false).implicit_value(true).help(
        "enable cuda for neural networks");

    /*
     * Train parser
     */

    argparse::ArgumentParser train_parser("train");

    train_parser.add_argument("output_path")
        .required()
        .help("output folder path (for models, metrics, etc)");

    train_parser.add_argument("-e", "--episodes")
        .scan<'i', int>()
        .default_value(1000)
        .help("episode number per save");

    train_parser.add_argument("-n", "--nb_saves")
        .scan<'i', int>()
        .default_value(1000)
        .help("number of save when training");

    /*
     * Run parser
     */

    argparse::ArgumentParser run_parser("run");

    run_parser.add_argument("input_folder").required().help("input folder containing .th file");

    run_parser.add_argument("-w", "--width")
        .scan<'i', int>()
        .default_value(1024)
        .help("window width");

    run_parser.add_argument("-h", "--height")
        .scan<'i', int>()
        .default_value(1024)
        .help("window height");

    cli_parser.add_subparser(run_parser);
    cli_parser.add_subparser(train_parser);

    parser.add_subparser(cli_parser);
    parser.add_subparser(gui_parser);

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }
    if (parser.is_subcommand_used(cli_parser)) {
        const auto agent_parameters =
            cli_parser.get<std::vector<std::pair<std::string, std::string>>>("agent_parameters");
        const auto agent_factory = get_agent_factory(
            cli_parser.get<std::string>("agent"),
            std::map<std::string, std::string>(agent_parameters.begin(), agent_parameters.end()));

        const auto env_parameters =
            cli_parser.get<std::vector<std::pair<std::string, std::string>>>("env_parameters");
        const auto env_factory = get_environment_factory(
            cli_parser.get<std::string>("environment"),
            std::map<std::string, std::string>(env_parameters.begin(), env_parameters.end()));

        if (cli_parser.is_subcommand_used(train_parser))
            train(
                cli_parser.get<int>("env_num_threads"), cli_parser.get<int>("env_seed"),
                cli_parser.get<bool>("cuda"),
                {train_parser.get<std::string>("output_path"), train_parser.get<int>("nb_saves"),
                 train_parser.get<int>("episodes")},
                agent_factory, env_factory);
        else if (cli_parser.is_subcommand_used(run_parser))
            infer(
                cli_parser.get<int>("env_num_threads"), cli_parser.get<int>("env_seed"),
                cli_parser.get<bool>("cuda"),
                {run_parser.get<std::string>("input_folder"), run_parser.get<int>("width"),
                 run_parser.get<int>("height")},
                agent_factory, env_factory);
        else {
            std::cerr << "must enter a valid subcommand" << std::endl
                      << cli_parser.help().str() << std::endl;
            exit(1);
        }
    } else if (parser.is_subcommand_used(gui_parser)) {
        gui();
    } else {
        std::cerr << "must enter a valid subcommand" << std::endl
                  << parser.help().str() << std::endl;
    }

    return 0;
}