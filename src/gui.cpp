//
// Created by samuel on 09/12/24.
//

#include <evo_motion_view/renderer.h>

#include <evo_motion_model/robot/builder.h>

#include "./run.h"

void gui() {
    RobotBuilderEnvironment env;
    env.load_robot("/home/samuel/CLionProjects/EvoMotion/evo_motion_model/resources/skeleton/new_format_spider.json");
    env.save_robot("/home/samuel/PycharmProjects/EvoMotion_json_conversion/new_format_spider_binary-float.json", "spider");
    return;

    ImGuiRenderer renderer("evo_motion", 1920, 1080);

    while (!renderer.is_close()) {
        auto before = std::chrono::system_clock::now();

        renderer.draw();

        std::chrono::duration<double, std::milli> delta = std::chrono::system_clock::now() - before;

        std::this_thread::sleep_for(std::chrono::milliseconds(
            static_cast<long>(std::max(0., 1000. / 60. - delta.count()))));
    }
}