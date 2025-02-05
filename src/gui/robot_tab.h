//
// Created by samuel on 28/01/25.
//

#ifndef EVO_MOTION_ROBOT_TAB_H
#define EVO_MOTION_ROBOT_TAB_H

#include <string>

class RobotTab {
public:
    explicit RobotTab(std::string name);

    void set_name(const std::string &new_name);
    std::string get_name();

    void imgui_render();

private:
    std::string robot_name;
};

#endif//EVO_MOTION_ROBOT_TAB_H
