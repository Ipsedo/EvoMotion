//
// Created by samuel on 09/12/24.
//

#include <evo_motion_model/robot/builder.h>
#include <evo_motion_view/renderer.h>

#include "./_convert.h"
#include "./run.h"

void gui() {

    convert_tree_skeleton_to_graph_skeleton();

    RobotBuilderEnvironment env;
    env.load_robot("/home/samuel/CLionProjects/EvoMotion/evo_motion_model/resources/skeleton/_test/"
                   "new_format_spider_c++-binary-float.json");
    env.save_robot(
        "/home/samuel/CLionProjects/EvoMotion/evo_motion_model/resources/skeleton/_test/"
        "new_format_spider_c++-binary-float_cpy.json",
        "spider");
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