//
// Created by samuel on 06/02/25.
//

#ifndef EVO_MOTION_CONTEXT_H
#define EVO_MOTION_CONTEXT_H

#include <memory>
#include <optional>

#include <evo_motion_model/robot/builder.h>

class ItemFocusContext {
public:
    ItemFocusContext();

    std::optional<glm::vec3> get_focus_color(const std::string &item_name);
    void release_focus(const std::string &item_name);
    void focus(const std::string &item_name, const glm::vec3 &focus_color);

    void focus_black(const std::string &item_name);
    void focus_grey(const std::string &item_name);

private:
    std::map<std::string, glm::vec3> item_to_focus_color;
};

#endif//EVO_MOTION_CONTEXT_H
