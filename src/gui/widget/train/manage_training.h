//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_MANAGE_TRAINING_H
#define EVO_MOTION_MANAGE_TRAINING_H

#include "../window.h"

class ManageTrainingWindow final : public ImGuiWindow {
public:
    ManageTrainingWindow();

protected:
    void render_window_content(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<OpenGlWindow> &gl_window) override;
};

#endif//EVO_MOTION_MANAGE_TRAINING_H
