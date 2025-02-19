//
// Created by samuel on 13/02/25.
//

#ifndef EVO_MOTION_POPUP_H
#define EVO_MOTION_POPUP_H

#include "./window.h"

class PopUpWindow : public ImGuiWindow {
public:
    explicit PopUpWindow(const std::string &popup_name);

    void render_window(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<OpenGlWindow> &gl_window) override;

protected:
    void render_window_content(
        const std::shared_ptr<ItemFocusContext> &context,
        const std::shared_ptr<OpenGlWindow> &gl_window) override;

    virtual void render_popup_content(const std::shared_ptr<ItemFocusContext> &context) = 0;

    ImVec2 get_min_size() override;

private:
    std::string name;
    bool first_open;
};

#endif//EVO_MOTION_POPUP_H
