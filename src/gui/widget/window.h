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
    ImGuiWindow(const std::string &name);

    void open();
    void close();

    virtual void render_window(const std::shared_ptr<AppContext> &context);

    virtual ~ImGuiWindow();

protected:
    virtual void render_window_content(const std::shared_ptr<AppContext> &context) = 0;

private:
    std::string name;
    bool show;
};

/*
 * Robot
 */

class RobotInfoWindow : public ImGuiWindow {
public:
    RobotInfoWindow();

protected:
    void render_window_content(const std::shared_ptr<AppContext> &context) override;
};

/*
 * Member
 */

class NewMemberWindow : public ImGuiWindow {
public:
    NewMemberWindow();

protected:
    void render_window_content(const std::shared_ptr<AppContext> &context) override;
};

class MemberSettingsWindow : public ImGuiWindow {
public:
    MemberSettingsWindow();

protected:
    void render_window_content(const std::shared_ptr<AppContext> &context) override;
};

class MemberConstructToolsWindow : public ImGuiWindow {
public:
    MemberConstructToolsWindow();

protected:
    void render_window_content(const std::shared_ptr<AppContext> &context) override;
};

/*
 * Constraint
 */

class NewConstraintWindow : public ImGuiWindow {
public:
    NewConstraintWindow();

protected:
    void render_window_content(const std::shared_ptr<AppContext> &context) override;
};

class ConstraintSettingsWindow : public ImGuiWindow {
public:
    ConstraintSettingsWindow();

protected:
    void render_window_content(const std::shared_ptr<AppContext> &context) override;
};

class ConstraintConstructToolsWindow : public ImGuiWindow {
public:
    ConstraintConstructToolsWindow();

protected:
    void render_window_content(const std::shared_ptr<AppContext> &context) override;
};

/*
 * Infer
 */

class InferSettingsWindow : public ImGuiWindow {
public:
    InferSettingsWindow(const std::function<void(std::shared_ptr<OpenGlWindow>)> &on_start_infer);
    void render_window(const std::shared_ptr<AppContext> &context) override;

protected:
    void render_window_content(const std::shared_ptr<AppContext> &context) override;

private:
    std::function<void(std::shared_ptr<OpenGlWindow>)> on_start_infer;

    ImGui::FileBrowser robot_infer_file_dialog;
    ImGui::FileBrowser agent_infer_file_dialog;
};

/*
 *
 */

class StartTrainingWindow : public ImGuiWindow {
public:
    StartTrainingWindow();

protected:
    void render_window_content(const std::shared_ptr<AppContext> &context) override;
};

class ManageTrainingWindow : public ImGuiWindow {
public:
    ManageTrainingWindow();

protected:
    void render_window_content(const std::shared_ptr<AppContext> &context) override;
};

#endif//EVO_MOTION_WINDOW_H
