//
// Created by samuel on 13/11/24.
//

#ifndef SAVER_H
#define SAVER_H

#include <filesystem>
#include <string>

#include <torch/torch.h>

template<typename T>
void save_torch(
    const std::string &output_folder_path, const T &to_save, const std::string &file_name) {
    const std::filesystem::path path(output_folder_path);

    if (!std::filesystem::exists(path))
        throw std::runtime_error("Could not find " + output_folder_path);

    const auto file = path / file_name;
    torch::serialize::OutputArchive archive;
    to_save->save(archive);
    archive.save_to(file);
}

template<typename T>
void load_torch(
    const std::string &input_folder_path, const T &to_fill, const std::string &file_name) {
    const std::filesystem::path path(input_folder_path);
    const auto file = path / file_name;

    if (!std::filesystem::exists(file))
        throw std::runtime_error("Could not find " + input_folder_path);

    torch::serialize::InputArchive archive;
    archive.load_from(file);
    to_fill->load(archive);
}

#endif//SAVER_H
