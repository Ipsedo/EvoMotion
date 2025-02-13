//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_MEMBER_CONSTRUCT_TOOLS_H
#define EVO_MOTION_MEMBER_CONSTRUCT_TOOLS_H

#include "../window.h"

class MemberConstructToolsWindow final : public ImGuiWindow {
public:
    MemberConstructToolsWindow();

protected:
    void render_window_content(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;
};

#endif//EVO_MOTION_MEMBER_CONSTRUCT_TOOLS_H
