//
// Created by samuel on 06/02/25.
//

#include "./context.h"

/*
 * Context
 */

AppContext::AppContext()
    : focused_member(), focused_constraint(), builder_env(), constraint_parent(),
      constraint_child(), members_hidden(false), constraints_hidden(true) {}

void AppContext::hide_members(const bool hidden) { members_hidden = hidden; }
bool AppContext::are_members_hidden() const { return members_hidden; }

void AppContext::hide_constraints(const bool hidden) { constraints_hidden = hidden; }
bool AppContext::are_constraints_hidden() const { return constraints_hidden; }

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
