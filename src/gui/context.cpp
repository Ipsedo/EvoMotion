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
