//
// Created by samuel on 09/12/24.
//

#include "./application.h"

#include <iostream>

#include <evo_motion_networks/agents/cross_q.h>

#include "./widget/constraint/constraint_popup.h"
#include "./widget/member/member_popup.h"

ImGuiApplication::ImGuiApplication(const std::string &title, const int width, const int height)
    : need_close(false), clear_color(0.45f, 0.55f, 0.60f, 1.00f),
      context(std::make_shared<ItemFocusContext>()), imgui_windows(), opengl_windows(),
      robot_builder_file_dialog(), popup_already_opened_robot("Popup_robot_already_opened"),
      part_kind(MEMBER), vao(0) {

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed" << std::endl;
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if (!window) {
        std::cerr << "GLFW window initialization failed" << std::endl;
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed" << std::endl;
        glfwTerminate();
        exit(1);
    }

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // file dialog
    robot_builder_file_dialog.SetTitle("Load robot JSON");
    robot_builder_file_dialog.SetTypeFilters({".json"});

    // OpenGL options
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    glDepthFunc(GL_LEQUAL);

    glDepthMask(GL_TRUE);

    glGenVertexArrays(1, &vao);

    glDebugMessageCallback(ImGuiApplication::message_callback, nullptr);
}

void ImGuiApplication::render() {
    glfwPollEvents();

    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // get active imgui window
    std::optional<std::string> active_opengl_window = std::nullopt;
    for (const auto &gl_window: opengl_windows)
        if (gl_window->is_active()) active_opengl_window = gl_window->get_name();

    if (active_opengl_window.has_value()) {
        // add new windows
        for (int i = imgui_windows[active_opengl_window.value()].size() - 1; i >= 0; i--) {
            const auto curr_window = imgui_windows[active_opengl_window.value()][i];
            auto w = curr_window->pop_child();
            while (w.has_value()) {
                if (!contains_window(
                        imgui_windows[active_opengl_window.value()], w.value()->get_name()))
                    imgui_windows[active_opengl_window.value()].push_back(w.value());
                w = curr_window->pop_child();
            }
        }

        // remove closed windows
        std::erase_if(imgui_windows[active_opengl_window.value()], [](const auto &w) {
            return w->is_closed();
        });

        // render them
        for (const auto &w: imgui_windows[active_opengl_window.value()]) w->render_window(context);
    }

    imgui_render_toolbar();
    imgui_render_robot_builder_file_dialog();

    imgui_render_opengl();

    // Pop-ups
    if (ImGui::BeginPopupModal(
            popup_already_opened_robot.c_str(), nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::Text("Robot already opened");

        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    // End - render
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);

    std::erase_if(opengl_windows, [](const auto &gl_window) { return !gl_window->is_opened(); });
}

void ImGuiApplication::imgui_render_toolbar() {
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Settings")) {}
            if (ImGui::MenuItem("Exit")) need_close = true;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Robot")) {

            if (ImGui::MenuItem("New robot")) {
                const auto new_robot = std::make_shared<RobotBuilderEnvironment>(
                    "robot_" + std::to_string(opengl_windows.size()));

                opengl_windows.push_back(create_builder_opengl_window(new_robot));

                imgui_windows[opengl_windows.back()->get_name()] = {};
            }

            if (ImGui::MenuItem("Load robot")) robot_builder_file_dialog.Open();

            if (ImGui::MenuItem("Save robot", nullptr, nullptr)) {}

            ImGui::Separator();

            if (ImGui::BeginMenu("Edit mode")) {
                if (ImGui::MenuItem("Members", nullptr, part_kind == MEMBER)) part_kind = MEMBER;
                if (ImGui::MenuItem("Constraints", nullptr, part_kind == CONSTRAINT))
                    part_kind = CONSTRAINT;
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Algorithm")) {
            if (ImGui::BeginMenu("Train")) {
                if (ImGui::MenuItem("Start training")) {}
                if (ImGui::MenuItem("Manage trainings")) {}
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Infer")) {}

            if (ImGui::MenuItem("Test / Debug")) { /* TODO run debug agent on robot */
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void ImGuiApplication::imgui_render_robot_builder_file_dialog() {
    robot_builder_file_dialog.Display();

    if (robot_builder_file_dialog.HasSelected()) {
        const std::filesystem::path robot_json_path = robot_builder_file_dialog.GetSelected();

        const auto robot = std::make_shared<RobotBuilderEnvironment>("");
        robot->load_robot(robot_json_path);

        if (!std::ranges::any_of(
                opengl_windows, [robot](const std::shared_ptr<OpenGlWindow> &gl_window) {
                    return gl_window->get_name() == robot->get_robot_name();
                })) {

            opengl_windows.push_back(create_builder_opengl_window(robot));
        } else {
            ImGui::OpenPopup(popup_already_opened_robot.c_str());
        }

        robot_builder_file_dialog.ClearSelected();
    }
}

void ImGuiApplication::imgui_render_opengl() {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImVec2 window_pos = viewport->Pos;
    ImVec2 available_size = viewport->Size;

    const float menu_bar_height = ImGui::GetFrameHeight();
    window_pos.y += menu_bar_height;
    available_size.y -= menu_bar_height;

    // Create the full-space background window
    ImGui::SetNextWindowPos(window_pos);
    ImGui::SetNextWindowSize(available_size);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoDecoration;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("OpenGL window", nullptr, flags);

    if (ImGui::BeginTabBar(
            "OpenGL tab bar", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_NoTooltip)) {
        const auto opengl_render_size = ImGui::GetContentRegionAvail();

        for (const auto &gl_win: opengl_windows) {
            if (gl_win->is_active()) {
                glBindVertexArray(vao);
                gl_win->draw_opengl(opengl_render_size.x, opengl_render_size.y);
            }
            gl_win->draw_imgui_image();
        }

        ImGui::EndTabBar();
    }

    ImGui::PopStyleVar(3);

    ImGui::End();
}

bool ImGuiApplication::is_close() const { return need_close || glfwWindowShouldClose(window); }

ImGuiApplication::~ImGuiApplication() {
    glDeleteVertexArrays(1, &vao);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void GLAPIENTRY ImGuiApplication::message_callback(
    const GLenum source, const GLenum type, const GLuint id, const GLenum severity,
    const GLsizei length, const GLchar *message, const void *userParam) {
    std::cerr << source << " " << type << " " << id << " " << severity << " " << length << " : "
              << std::endl;
    std::cerr << "params : " << userParam << std::endl;
    std::cerr << message << std::endl << std::endl;
}

std::shared_ptr<BuilderOpenGlWindow> ImGuiApplication::create_builder_opengl_window(
    const std::shared_ptr<RobotBuilderEnvironment> &builder_env) {
    return std::make_shared<BuilderOpenGlWindow>(
        context, builder_env->get_robot_name(), builder_env,
        [this](
            const std::string &gl_window_name, std::optional<std::string> focused_member,
            std::shared_ptr<RobotBuilderEnvironment> curr_builder_env) {
            if (focused_member.has_value()
                && !contains_window(imgui_windows[gl_window_name], focused_member.value()))
                imgui_windows[gl_window_name].push_back(std::make_shared<FocusMemberPopUpWindow>(
                    focused_member.value(), curr_builder_env));

            if (!focused_member.has_value())
                imgui_windows[gl_window_name].push_back(
                    std::make_shared<NoFocusMemberPopUpWindow>(curr_builder_env));
        },
        [this](
            const std::string &gl_window_name, std::optional<std::string> focused_constraint,
            std::shared_ptr<RobotBuilderEnvironment> curr_builder_env) {
            if (focused_constraint.has_value()
                && !contains_window(imgui_windows[gl_window_name], focused_constraint.value()))
                imgui_windows[gl_window_name].push_back(
                    std::make_shared<FocusConstraintPopUpWindow>(
                        focused_constraint.value(), curr_builder_env));

            if (!focused_constraint.has_value())
                imgui_windows[gl_window_name].push_back(
                    std::make_shared<NoFocusConstraintPopUpWindow>(curr_builder_env));
        },
        [this]() { return part_kind; });
}

bool ImGuiApplication::contains_window(
    const std::vector<std::shared_ptr<ImGuiWindow>> &windows, const std::string &window_name) {
    return std::ranges::any_of(
        windows, [window_name](const auto &w) { return w->get_name() == window_name; });
}
