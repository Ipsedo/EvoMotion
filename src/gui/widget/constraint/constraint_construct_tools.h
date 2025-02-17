//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_CONSTRAINT_CONSTRUCT_TOOLS_H
#define EVO_MOTION_CONSTRAINT_CONSTRUCT_TOOLS_H

#include "../window.h"

class ConstraintConstructToolsWindow final : public ImGuiWindow {
public:
    ConstraintConstructToolsWindow();

protected:
    void render_window_content(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<OpenGlWindow> &gl_window) override;
};

#endif//EVO_MOTION_CONSTRAINT_CONSTRUCT_TOOLS_H
