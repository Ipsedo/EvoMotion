//
// Created by samuel on 13/02/25.
//

#ifndef EVO_MOTION_POPUP_H
#define EVO_MOTION_POPUP_H

#include "./window.h"

class PopUpWindow : public ImGuiWindow {
public:
    PopUpWindow(const std::string &popup_name);

    void render_window(const std::shared_ptr<ItemFocusContext> &context) override;

protected:
    void render_window_content(const std::shared_ptr<ItemFocusContext> &context) override;

private:
    std::string name;
    bool first_open;
};

#endif//EVO_MOTION_POPUP_H
