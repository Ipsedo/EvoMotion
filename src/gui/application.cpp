//
// Created by samuel on 09/12/24.
//

#include "./application.h"

#include <iostream>

#include <evo_motion_networks/agents/cross_q.h>

ImGuiApplication::ImGuiApplication(const std::string &title, const int width, const int height)
    : need_close(false), clear_color(0.45f, 0.55f, 0.60f, 1.00f), show_new_member_window(false),
      show_member_settings_window(false), show_new_constraint_window(false),
      show_constraint_settings_window(false), show_member_construct_tools_window(false),
      show_constraint_construct_tools_window(false), show_manage_trainings_window(false),
      show_new_training_window(false), show_robot_info_window(false), show_infer_window(false),
      context(std::make_shared<AppContext>()), opengl_windows(), robot_builder_file_dialog(),
      robot_infer_file_dialog(), agent_infer_file_dialog(ImGuiFileBrowserFlags_SelectDirectory),
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
        show_member_construct_tools_window = false;
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
    imgui_render_new_member();
    imgui_render_member_settings();
    imgui_render_construct_member_tools();
    imgui_render_new_constraint();
    imgui_render_constraint_settings();
    imgui_render_construct_constraint_tools();
    imgui_render_robot_info();
    imgui_render_robot_builder_file_dialog();

    imgui_render_new_training();
    imgui_render_manage_trainings();

    imgui_render_robot_infer();
    imgui_render_agent_infer_file_dialog();
    imgui_render_robot_infer_file_dialog();

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

            if (ImGui::BeginMenu("Show parts", context->is_builder_env_selected())) {
                if (ImGui::MenuItem("Members", nullptr, !context->are_members_hidden())) {
                    context->hide_constraints(true);
                    context->release_focus_constraint();

                    context->hide_members(false);
                }
                if (ImGui::MenuItem("Constraints", nullptr, !context->are_constraints_hidden())) {
                    context->hide_members(true);
                    context->release_focus_member();

                    context->hide_constraints(false);
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Member", context->is_builder_env_selected())) {
                if (ImGui::MenuItem("New member")) show_new_member_window = true;
                if (ImGui::MenuItem("Settings")) show_member_settings_window = true;
                if (ImGui::MenuItem("Construct tools")) show_member_construct_tools_window = true;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Constraint", context->is_builder_env_selected())) {
                if (ImGui::MenuItem("New constraint")) show_new_constraint_window = true;
                if (ImGui::MenuItem("Settings")) show_constraint_settings_window = true;
                if (ImGui::MenuItem("Construct tools"))
                    show_constraint_construct_tools_window = true;

                ImGui::EndMenu();
            }

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
            if (context->is_member_focused()) {
                ImGui::Text("Focus on \"%s\" member", context->get_focused_member().c_str());

                auto [member_pos, member_rot, member_scale] =
                    context->get_builder_env()->get_member_transform(context->get_focused_member());

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Columns(2, nullptr, false);

                // Position
                ImGui::Spacing();

                ImGui::BeginGroup();
                ImGui::Text("Position");
                ImGui::Spacing();

                bool updated = false;

                if (ImGui::InputFloat("pos.x", &member_pos.x, 0.f, 0.f, "%.8f")) updated = true;
                if (ImGui::InputFloat("pos.y", &member_pos.y, 0.f, 0.f, "%.8f")) updated = true;
                if (ImGui::InputFloat("pos.z", &member_pos.z, 0.f, 0.f, "%.8f")) updated = true;

                ImGui::EndGroup();

                // Rotation
                ImGui::Spacing();

                ImGui::BeginGroup();

                ImGui::Text("Rotation quaternion");
                ImGui::Spacing();

                if (ImGui::InputFloat("quat.w", &member_rot.w, 0.f, 0.f, "%.8f")) updated = true;
                if (ImGui::InputFloat("quat.x", &member_rot.x, 0.f, 0.f, "%.8f")) updated = true;
                if (ImGui::InputFloat("quat.y", &member_rot.y, 0.f, 0.f, "%.8f")) updated = true;
                if (ImGui::InputFloat("quat.z", &member_rot.z, 0.f, 0.f, "%.8f")) updated = true;

                ImGui::EndGroup();

                // Scaling
                ImGui::Spacing();

                ImGui::BeginGroup();

                ImGui::Text("Scale");
                ImGui::Spacing();

                if (ImGui::InputFloat("scale.x", &member_scale.x, 0.f, 0.f, "%.8f")) updated = true;
                if (ImGui::InputFloat("scale.y", &member_scale.y, 0.f, 0.f, "%.8f")) updated = true;
                if (ImGui::InputFloat("scale.z", &member_scale.z, 0.f, 0.f, "%.8f")) updated = true;

                ImGui::EndGroup();

                ImGui::NextColumn();

                // Member name
                ImGui::Spacing();
                ImGui::Text("Member parameters");
                ImGui::Spacing();

                // Member name
                std::string member_name = context->get_focused_member();
                member_name.resize(128);
                if (ImGui::InputText("Member name", &member_name[0], member_name.size()))
                    if (context->get_builder_env()->rename_member(
                            context->get_focused_member(), member_name.c_str()))
                        context->set_focus_member(member_name.c_str());

                ImGui::Spacing();

                // mass
                float mass =
                    context->get_builder_env()->get_member_mass(context->get_focused_member());
                if (ImGui::InputFloat("mass (kg)", &mass, 0.f, 0.f, "%.8f")) updated = true;

                ImGui::Spacing();

                // friction
                float friction =
                    context->get_builder_env()->get_member_friction(context->get_focused_member());
                if (ImGui::DragFloat("friction", &friction, 0.01f, 0.f, 1.f)) updated = true;

                ImGui::Columns(1);

                if (updated)
                    context->get_builder_env()->update_member(
                        context->get_focused_member(), member_pos, member_rot, member_scale,
                        friction, mass);

                // remove
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Remove member")) {
                    context->get_builder_env()->remove_member(context->get_focused_member());
                    context->release_focus_member();
                }

            } else {
                ImGui::Text("No focus member");
            }
        }
        ImGui::End();
    }
}

void ImGuiApplication::imgui_render_new_member() {
    if (show_new_member_window) {
        if (ImGui::Begin("New member", &show_new_member_window)) {}
        ImGui::End();
    }
}

void ImGuiApplication::imgui_render_new_constraint() {
    if (show_new_constraint_window) {
        if (ImGui::Begin("New constraint", &show_new_constraint_window)) {}
        ImGui::End();
    }
}

void ImGuiApplication::imgui_render_constraint_settings() {
    if (show_constraint_settings_window) {
        if (ImGui::Begin("Constraint settings", &show_constraint_settings_window)) {
            if (context->is_constraint_focused()) {
                ImGui::Text(
                    "Focus on \"%s\" constraint", context->get_focused_constraint().c_str());

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                const auto constraint_type = context->get_builder_env()->get_constraint_type(
                    context->get_focused_constraint());

                if (constraint_type == HINGE) {
                    ImGui::Text("Hinge constraint");

                    bool updated = false;

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    auto [pos, axis, limit_angle_min, limit_angle_max] =
                        context->get_builder_env()->get_constraint_hinge_info(
                            context->get_focused_constraint());

                    // position
                    ImGui::BeginGroup();
                    ImGui::Text("Position");
                    ImGui::Spacing();

                    if (ImGui::InputFloat("pos.x", &pos.x, 0.f, 0.f, "%.8f")) updated = true;
                    if (ImGui::InputFloat("pos.y", &pos.y, 0.f, 0.f, "%.8f")) updated = true;
                    if (ImGui::InputFloat("pos.z", &pos.z, 0.f, 0.f, "%.8f")) updated = true;

                    ImGui::EndGroup();
                    ImGui::Spacing();

                    // axis
                    ImGui::BeginGroup();
                    ImGui::Text("Axis");
                    ImGui::Spacing();

                    if (ImGui::InputFloat("axis.x", &axis.x, 0.f, 0.f, "%.8f")) updated = true;
                    if (ImGui::InputFloat("axis.y", &axis.y, 0.f, 0.f, "%.8f")) updated = true;
                    if (ImGui::InputFloat("axis.z", &axis.z, 0.f, 0.f, "%.8f")) updated = true;

                    ImGui::EndGroup();
                    ImGui::Spacing();

                    // Limit
                    ImGui::BeginGroup();
                    ImGui::Text("Angular limits");
                    ImGui::Spacing();

                    if (ImGui::InputFloat("min", &limit_angle_min, 0.f, 0.f, "%.8f"))
                        updated = true;
                    if (ImGui::InputFloat("max", &limit_angle_max, 0.f, 0.f, "%.8f"))
                        updated = true;

                    ImGui::EndGroup();

                    // final
                    if (updated)
                        context->get_builder_env()->update_hinge_constraint(
                            context->get_focused_constraint(), pos, axis, limit_angle_min,
                            limit_angle_max);

                } else if (constraint_type == FIXED) {
                    ImGui::Text("Fixed constraint");

                    bool updated = false;

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    auto [pos, rot] = context->get_builder_env()->get_constraint_fixed_info(
                        context->get_focused_constraint());

                    // position
                    ImGui::BeginGroup();
                    ImGui::Text("Position");
                    ImGui::Spacing();

                    if (ImGui::InputFloat("pos.x", &pos.x, 0.f, 0.f, "%.8f")) updated = true;
                    if (ImGui::InputFloat("pos.y", &pos.y, 0.f, 0.f, "%.8f")) updated = true;
                    if (ImGui::InputFloat("pos.z", &pos.z, 0.f, 0.f, "%.8f")) updated = true;

                    ImGui::EndGroup();
                    ImGui::Spacing();

                    // axis
                    ImGui::BeginGroup();
                    ImGui::Text("Rotation");
                    ImGui::Spacing();

                    if (ImGui::InputFloat("axis.w", &rot.w, 0.f, 0.f, "%.8f")) updated = true;
                    if (ImGui::InputFloat("axis.x", &rot.x, 0.f, 0.f, "%.8f")) updated = true;
                    if (ImGui::InputFloat("axis.y", &rot.y, 0.f, 0.f, "%.8f")) updated = true;
                    if (ImGui::InputFloat("axis.z", &rot.z, 0.f, 0.f, "%.8f")) updated = true;

                    ImGui::EndGroup();
                    ImGui::Spacing();

                    if (updated)
                        context->get_builder_env()->update_fixed_constraint(
                            context->get_focused_constraint(), pos, rot);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Remove constraint")) {
                    context->get_builder_env()->remove_constraint(
                        context->get_focused_constraint());
                    context->release_focus_constraint();
                }

            } else {
                ImGui::Text("No focused constraint");
            }
        }
        ImGui::End();
    }
}

void ImGuiApplication::imgui_render_construct_member_tools() {
    if (show_member_construct_tools_window) {
        if (ImGui::Begin("Construct tools - Member", &show_member_construct_tools_window)) {}
        ImGui::End();
    }
}

void ImGuiApplication::imgui_render_construct_constraint_tools() {
    if (show_constraint_construct_tools_window) {
        if (ImGui::Begin("Construct tools - Constraint", &show_constraint_construct_tools_window)) {
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
            ImGui::Text("Members count : %i", context->get_builder_env()->get_members_count());

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

            std::vector<std::string> item_names = context->get_builder_env()->get_member_names();
            const auto root_name = context->get_builder_env()->get_root_name();

            for (int i = 0; i < item_names.size(); i++)
                if (item_names[i] == root_name) {
                    selected_item = i;
                    break;
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