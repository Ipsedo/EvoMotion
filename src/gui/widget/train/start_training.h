//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_START_TRAINING_H
#define EVO_MOTION_START_TRAINING_H

#include "../window.h"

class StartTrainingWindow final : public ImGuiWindow {
public:
    StartTrainingWindow();

protected:
    void render_window_content(const std::shared_ptr<ItemFocusContext> &context) override;
};

#endif//EVO_MOTION_START_TRAINING_H
