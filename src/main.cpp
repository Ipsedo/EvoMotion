//
// Created by samuel on 15/12/22.
//

#include <iostream>
#include <memory>

#include <argparse/argparse.hpp>

#include <evo_motion_networks/agent.h>

#include "./run.h"

int main(const int argc, char **argv) {
    argparse::ArgumentParser parser("evo_motion");

    parser.add_argument("environment").default_value("cartpole").help("the environment");
    parser.add_argument("agent").default_value("actor_critic_liquid").help("the agent");

    parser.add_argument("-p", "--parameters")
        .append()
        .action([](const std::string &k_v) {
            const auto delimiter = '=';

            if (std::count(k_v.begin(), k_v.end(), delimiter) != 1)
                throw std::invalid_argument(k_v);

            const std::string key = k_v.substr(0, k_v.find(delimiter));
            const std::string value = k_v.substr(k_v.find(delimiter) + 1);

            return std::pair(key, value);
        })
        .help("agent hyper-parameters");

    parser.add_argument("--env_seed")
        .scan<'i', int>()
        .default_value(1234)
        .help("seed for environment RNG");

    parser.add_argument("--cuda").default_value(false).implicit_value(true).help(
        "enable cuda for neural networks");

    /*parser.add_argument("--hidden_size")
        .default_value(32)
        .scan<'i', int>()
        .help("neural network hidden size");*/

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

    /*train_parser.add_argument("-b", "--batch_size")
        .scan<'i', int>()
        .default_value(16)
        .help("episodes batch size");

    train_parser.add_argument("-l", "--learning_rate")
        .scan<'g', float>()
        .default_value(1e-4f)
        .help("agent learning rate");*/

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

    parser.add_subparser(run_parser);
    parser.add_subparser(train_parser);

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    const auto parameters =
        parser.get<std::vector<std::pair<std::string, std::string>>>("parameters");
    const auto agent_factory = get_factory(
        parser.get<std::string>("agent"),
        std::map<std::string, std::string>(parameters.begin(), parameters.end()));

    if (parser.is_subcommand_used(train_parser))
        train(
            parser.get<int>("env_seed"), parser.get<bool>("cuda"),
            {parser.get<std::string>("environment"), train_parser.get<std::string>("output_path"),
             train_parser.get<int>("nb_saves"), train_parser.get<int>("episodes")},
            agent_factory);
    else if (parser.is_subcommand_used(run_parser))
        infer(
            parser.get<int>("env_seed"), parser.get<bool>("cuda"),
            {parser.get<std::string>("environment"), run_parser.get<std::string>("input_folder"),
             run_parser.get<int>("width"), run_parser.get<int>("height")},
            agent_factory);
    else {
        std::cerr << "must enter a subcommand" << std::endl << parser.help().str() << std::endl;
        exit(1);
    }

    return 0;
}