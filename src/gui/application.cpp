//
// Created by samuel on 09/12/24.
//

#include "./application.h"

#include <iostream>

#include <evo_motion_networks/agents/cross_q.h>

ImGuiApplication::ImGuiApplication(const std::string &title, const int width, const int height)
    : need_close(false), clear_color(0.45f, 0.55f, 0.60f, 1.00f),
      show_member_settings_window(false), show_construct_tools_window(false),
      show_manage_trainings_window(false), show_new_training_window(false),
      show_robot_info_window(false), show_infer_window(false),
      context(std::make_shared<AppContext>()), opengl_windows(), robot_builder_file_dialog(),
      robot_infer_file_dialog(), agent_infer_file_dialog(ImGuiFileBrowserFlags_SelectDirectory),
      opengl_render_size(static_cast<float>(width), static_cast<float>(height)),
      popup_already_opened_robot("Popup_robot_already_opened"), vao(0) {

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

    robot_infer_file_dialog.SetTitle("Load robot JSON");
    robot_infer_file_dialog.SetTypeFilters({".json"});

    agent_infer_file_dialog.SetTitle("Load agent directory");

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
    if (!context->is_builder_env_selected()) {
        show_member_settings_window = false;
        show_construct_tools_window = false;
        show_robot_info_window = false;
    }

    glfwPollEvents();

    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    imgui_render_toolbar();

    // ImGui window definition
    imgui_render_member_settings();
    imgui_render_robot_info();
    imgui_render_robot_builder_file_dialog();
    imgui_render_construct_tools();

    imgui_render_new_training();
    imgui_render_manage_trainings();

    imgui_render_robot_infer();
    imgui_render_agent_infer_file_dialog();
    imgui_render_robot_infer_file_dialog();

    for (const auto &gl_window: opengl_windows)
        if (gl_window->is_active()) {
            glBindVertexArray(vao);
            gl_window->draw_opengl(opengl_render_size.x, opengl_render_size.y);
        }

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

        if (ImGui::BeginMenu("Robots")) {

            if (ImGui::MenuItem("New robot")) {

                auto new_robot = std::make_shared<RobotBuilderEnvironment>(
                    "robot_" + std::to_string(opengl_windows.size()));

                opengl_windows.push_back(std::make_shared<BuilderOpenGlWindow>(
                    context, new_robot->get_robot_name(), new_robot));
            }

            if (ImGui::MenuItem("Load robot")) robot_builder_file_dialog.Open();

            if (ImGui::MenuItem(
                    "Save robot", nullptr, nullptr, context->is_builder_env_selected())) {}

            ImGui::Separator();

            if (ImGui::MenuItem(
                    "Robot information", nullptr, false, context->is_builder_env_selected()))
                show_robot_info_window = true;

            ImGui::Separator();

            if (ImGui::MenuItem(
                    "Member settings", nullptr, false, context->is_builder_env_selected()))
                show_member_settings_window = true;
            if (ImGui::MenuItem(
                    "Construct tools", nullptr, false, context->is_builder_env_selected()))
                show_construct_tools_window = true;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Algorithm")) {
            if (ImGui::BeginMenu("Train")) {
                if (ImGui::MenuItem("Start training")) show_new_training_window = true;
                if (ImGui::MenuItem("Manage trainings")) show_manage_trainings_window = true;
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Infer")) show_infer_window = true;

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

            opengl_windows.push_back(
                std::make_shared<BuilderOpenGlWindow>(context, robot->get_robot_name(), robot));
        } else {
            ImGui::OpenPopup(popup_already_opened_robot.c_str());
        }

        robot_builder_file_dialog.ClearSelected();
    }
}

void ImGuiApplication::imgui_render_member_settings() {
    if (show_member_settings_window) {
        if (ImGui::Begin("Member settings", &show_member_settings_window)) {
            std::string message = "No focus member";

            if (context->is_member_focused()) {
                ImGui::Text("Focus on \"%s\" member", context->get_focused_member().c_str());

                auto [member_pos, member_rot, member_scale] =
                    context->get_builder_env()->get_member_transform(context->get_focused_member());

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Position");
                ImGui::Spacing();

                bool updated = false;

                if (ImGui::InputFloat("pos.x", &member_pos.x)) updated = true;
                if (ImGui::InputFloat("pos.y", &member_pos.y)) updated = true;
                if (ImGui::InputFloat("pos.z", &member_pos.z)) updated = true;

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Rotation quaternion");
                ImGui::Spacing();

                if (ImGui::InputFloat("quat.w", &member_rot.w)) updated = true;
                if (ImGui::InputFloat("quat.x", &member_rot.x)) updated = true;
                if (ImGui::InputFloat("quat.y", &member_rot.y)) updated = true;
                if (ImGui::InputFloat("quat.z", &member_rot.z)) updated = true;

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Scale");
                ImGui::Spacing();

                if (ImGui::InputFloat("scale.x", &member_scale.x)) updated = true;
                if (ImGui::InputFloat("scale.y", &member_scale.y)) updated = true;
                if (ImGui::InputFloat("scale.z", &member_scale.z)) updated = true;

                if (updated)
                    context->get_builder_env()->update_member(
                        context->get_focused_member(), member_pos, member_rot, member_scale);

            } else {
                ImGui::Text("No focus member");
            }
        }
        ImGui::End();
    }
}

void ImGuiApplication::imgui_render_robot_info() {
    if (show_robot_info_window) {
        if (ImGui::Begin("Robot information", &show_robot_info_window)) {

            ImGui::Spacing();

            ImGui::Text(
                "Robot selected : %s", context->get_builder_env()->get_robot_name().c_str());
            ImGui::Text(
                "Member selected : %s",
                context->is_member_focused() ? context->get_focused_member().c_str() : "no member");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Root member : %s", context->get_builder_env()->get_root_name().c_str());
            ImGui::Text(
                "Members count : %i",
                static_cast<int>(context->get_builder_env()->get_items().size()));

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // robot name
            std::string robot_name = context->get_builder_env()->get_robot_name();
            robot_name.resize(128);
            if (ImGui::InputText("Robot name", &robot_name[0], robot_name.size()))
                context->get_builder_env()->set_robot_name(robot_name.c_str());

            // select root item
            int selected_item = 0;
            std::vector<std::string> item_names;

            const auto items = context->get_builder_env()->get_items();
            const auto root_name = context->get_builder_env()->get_root_name();

            for (int i = 0; i < items.size(); i++) {
                if (items[i].get_name() == root_name) selected_item = i;
                item_names.push_back(items[i].get_name());
            }

            if (ImGui::Combo(
                    "Select root item", &selected_item,
                    [](void *user_ptr, int idx, const char **out_text) {
                        auto &vec = *static_cast<std::vector<std::string> *>(user_ptr);
                        if (idx < 0 || idx >= static_cast<int>(vec.size())) return false;
                        *out_text = vec[idx].c_str();
                        return true;
                    },
                    &item_names, item_names.size()))
                context->get_builder_env()->set_root(item_names[selected_item]);
        }
        ImGui::End();
    }
}

void ImGuiApplication::imgui_render_construct_tools() {
    if (show_construct_tools_window) {
        if (ImGui::Begin("Construct tools", &show_construct_tools_window)) {}
        ImGui::End();
    }
}

void ImGuiApplication::imgui_render_new_training() {
    if (show_new_training_window) {
        if (ImGui::Begin("Start training", &show_new_training_window)) {}
        ImGui::End();
    }
}

void ImGuiApplication::imgui_render_manage_trainings() {
    if (show_manage_trainings_window) {
        if (ImGui::Begin("Manage trainings", &show_manage_trainings_window)) {}
        ImGui::End();
    }
}

void ImGuiApplication::imgui_render_robot_infer_file_dialog() {
    robot_infer_file_dialog.Display();

    if (robot_infer_file_dialog.HasSelected()) {
        context->set_robot_infer_json_path(robot_infer_file_dialog.GetSelected());
        robot_infer_file_dialog.ClearSelected();
    }
}

void ImGuiApplication::imgui_render_agent_infer_file_dialog() {
    agent_infer_file_dialog.Display();

    if (agent_infer_file_dialog.HasSelected()) {
        context->set_agent_infer_path(agent_infer_file_dialog.GetSelected());
        agent_infer_file_dialog.ClearSelected();
    }
}

void ImGuiApplication::imgui_render_robot_infer() {
    if (show_infer_window) {
        if (ImGui::Begin("Infer trained agent", &show_infer_window)) {

            ImGui::Spacing();

            // Robot
            std::string loaded_json_path = context->is_robot_infer_json_path_selected()
                                               ? context->get_robot_infer_json_path().string()
                                               : "No robot selected";
            loaded_json_path.resize(1024);

            if (ImGui::InputText("Load robot", &loaded_json_path[0], loaded_json_path.size()))
                context->set_robot_infer_json_path(std::filesystem::path(loaded_json_path.c_str()));

            ImGui::SameLine();
            if (ImGui::ArrowButton("Load robot button", ImGuiDir_Right))
                robot_infer_file_dialog.Open();

            ImGui::Spacing();

            // Agent
            std::string loaded_agent_path = context->is_agent_infer_path_selected()
                                                ? context->get_agent_infer_path().string()
                                                : "No agent selected";
            loaded_agent_path.resize(1024);

            if (ImGui::InputText("Load agent", &loaded_agent_path[0], loaded_agent_path.size()))
                context->set_agent_infer_path(std::filesystem::path(loaded_agent_path.c_str()));

            ImGui::SameLine();
            if (ImGui::ArrowButton("Load agent button", ImGuiDir_Right))
                agent_infer_file_dialog.Open();

            ImGui::Spacing();
            ImGui::Separator();

            if (ImGui::Button("Start inference") && context->is_agent_infer_path_selected()
                && context->is_robot_infer_json_path_selected()) {
                const auto env = get_environment_factory("robot_walk", {})->get_env(4, 1234);
                const auto agent = std::make_shared<CrossQAgent>(
                    12345, env->get_state_space(), env->get_action_space(), 256, 1024, 128, 1,
                    3e-4f, 0.99f, 1, 2);
                agent->load(context->get_agent_infer_path());// TODO check if loaded successfully
                agent->to(torch::kCPU);
                opengl_windows.push_back(std::make_shared<InferOpenGlWindow>(
                    agent, "Infer " + std::to_string(opengl_windows.size()), env));

                show_infer_window = false;
                context->release_agent_infer_path();
                context->release_robot_infer_json_path();
            }
        }
        ImGui::End();
    }
}

void ImGuiApplication::imgui_render_opengl() {
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImVec2 window_pos = viewport->Pos;
    ImVec2 available_size = viewport->Size;

    float menu_bar_height = ImGui::GetFrameHeight();
    window_pos.y += menu_bar_height;
    available_size.y -= menu_bar_height;

    // Create the full-space background window
    ImGui::SetNextWindowPos(window_pos);
    ImGui::SetNextWindowSize(available_size);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                             | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus
                             | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("OpenGL window", nullptr, flags);

    if (ImGui::BeginTabBar(
            "OpenGL tab bar", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_NoTooltip)) {
        opengl_render_size = ImGui::GetContentRegionAvail();

        for (const auto &gl_win: opengl_windows) gl_win->draw_imgui_image();

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