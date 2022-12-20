//
// Created by samuel on 15/12/22.
//

#include <filesystem>
#include <iostream>
#include <memory>

#include <argparse/argparse.hpp>

#include "./train.h"
#include "./run.h"

int main(int argc, char **argv) {
    argparse::ArgumentParser parser("evo_motion");

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

    train_parser.add_argument("-l", "--learning_rate")
            .scan<'g', float>()
            .default_value(1e-4f)
            .help("agent learning rate");

    /*
     * Run parser
     */

    argparse::ArgumentParser run_parser("run");

    run_parser.add_argument("input_folder")
            .required()
            .help("input folder containing .th file");

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
    }
    catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    if (parser.is_subcommand_used(train_parser))
        train(
                1234,
                train_parser.get<std::string>("output_path"),
                {
                        train_parser.get<float>("learning_rate"),
                        train_parser.get<int>("nb_saves"),
                        train_parser.get<int>("episodes")
                }
        );
    else if (parser.is_subcommand_used(run_parser))
        infer(1234,
              {
                      run_parser.get<std::string>("input_folder"),
                      run_parser.get<int>("width"),
                      run_parser.get<int>("height"),
              }
        );
    else {
        std::cerr << "must enter a subcommand" << std::endl << parser.help().str() << std::endl;
        exit(1);
    }

    return 0;
}