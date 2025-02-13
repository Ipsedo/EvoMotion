//
// Created by samuel on 06/02/25.
//

#include "./context.h"

/*
 * Focus Item Context
 */

ItemFocusContext::ItemFocusContext() : item_to_focus_color() {}

std::optional<glm::vec3> ItemFocusContext::get_focus_color(const std::string &item_name) {
    if (item_to_focus_color.contains(item_name)) return item_to_focus_color[item_name];
    return std::nullopt;
}

void ItemFocusContext::release_focus(const std::string &item_name) {
    if (item_to_focus_color.contains(item_name)) item_to_focus_color.erase(item_name);
}

void ItemFocusContext::focus(const std::string &item_name, const glm::vec3 &focus_color) {
    item_to_focus_color[item_name] = focus_color;
}

void ItemFocusContext::focus_black(const std::string &item_name) {
    focus(item_name, glm::vec3(0.f));
}

void ItemFocusContext::focus_grey(const std::string &item_name) {
    focus(item_name, glm::vec3(0.5f));
}

void ItemFocusContext::focus_white(const std::string &item_name) {
    focus(item_name, glm::vec3(1.f));
}
