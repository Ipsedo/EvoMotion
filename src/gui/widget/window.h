//
// Created by samuel on 11/02/25.
//

#ifndef EVO_MOTION_WINDOW_H
#define EVO_MOTION_WINDOW_H

#include <memory>

#include <imgui.h>

// clang-format off
#include <imfilebrowser.h>
// clang-format on

#include "../context.h"
#include "./opengl_window.h"

class ImGuiWindow {
public:
    explicit ImGuiWindow(std::string name);

    bool is_closed() const;
    void close();
    virtual void render_window(const std::shared_ptr<ItemFocusContext> &context);
    virtual std::optional<std::shared_ptr<ImGuiWindow>> pop_child();
    virtual ~ImGuiWindow();
    std::string get_name() const;

protected:
    virtual void render_window_content(const std::shared_ptr<ItemFocusContext> &context) = 0;
    virtual void on_close(const std::shared_ptr<ItemFocusContext> &context) = 0;
    virtual void
    on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) = 0;

    virtual ImVec2 get_min_size();

private:
    std::string name;
    bool show;
    bool focus;
};

#endif//EVO_MOTION_WINDOW_H
