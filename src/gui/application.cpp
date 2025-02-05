//
// Created by samuel on 09/12/24.
//

#include "./application.h"

#include <iostream>

ImGuiApplication::ImGuiApplication(const std::string &title, const int width, const int height)
    : need_close(false), clear_color(0.45f, 0.55f, 0.60f, 1.00f), show_member_window(false),
      show_construct_tools_window(false), show_training_window(true),
      curr_robot_builder_env(std::nullopt), member_focus(std::nullopt), opengl_windows(),
      robot_file_dialog(), rng(std::chrono::duration_cast<std::chrono::microseconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count()),
      rd_uni(0.f, 1.f), opengl_render_size(static_cast<float>(width), static_cast<float>(height)),
      popup_already_opened_robot("Popup_robot_already_opened") {

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed" << std::endl;
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if (!window) {
        std::cerr << "GLFW window initialization failed" << std::endl;
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);// Enable vsync

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed" << std::endl;
        glfwTerminate();
        exit(1);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    ImGui_ImplOpenGL3_Init("#version 130");

    robot_file_dialog.SetTitle("Load robot JSON");
    robot_file_dialog.SetTypeFilters({".json"});

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    glDepthFunc(GL_LEQUAL);

    glDepthMask(GL_TRUE);

    frame_buffer = std::make_unique<FrameBuffer>(width, height);
}

void ImGuiApplication::draw() {
    glfwPollEvents();

    for (const auto &gl_window: opengl_windows)
        if (gl_window->is_active())
            gl_window->draw_opengl(opengl_render_size.x, opengl_render_size.y);

    std::ranges::for_each(opengl_windows, [this](const auto &gl_window) {
        if (auto env = std::dynamic_pointer_cast<RobotBuilderEnvironment>(gl_window->get_env());
            gl_window->is_active() && env) {
            if (curr_robot_builder_env.has_value()
                && curr_robot_builder_env.value()->get_robot_name() != env->get_robot_name())
                member_focus = std::nullopt;
            curr_robot_builder_env = env;
        }
    });

    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ImGui window definition
    imgui_render_toolbar();
    imgui_render_opengl();
    imgui_render_construct_tools();
    imgui_render_file_dialog();

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
                opengl_windows.push_back(std::make_shared<OpenGlWindow>(
                    new_robot->get_robot_name(), new_robot,
                    [this, new_robot](const glm::vec3 &near, const glm::vec3 &far) {
                        member_focus = new_robot->ray_cast_member(near, far);
                    }));
                member_focus = std::nullopt;
                curr_robot_builder_env = new_robot;
            }

            if (ImGui::MenuItem("Load robot")) robot_file_dialog.Open();

            if (ImGui::BeginMenu("Loaded robots")) {
                bool empty = true;
                for (const auto &gl_window: opengl_windows) {
                    if (const auto env = std::dynamic_pointer_cast<RobotBuilderEnvironment>(
                            gl_window->get_env());
                        env) {
                        ImGui::MenuItem(
                            env->get_robot_name().c_str(), nullptr, gl_window->is_active(), false);
                        empty = false;
                    }
                }

                if (empty) {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 200, 200, 255));
                    ImGui::Text("No robot(s) loaded");
                    ImGui::PopStyleColor();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Robot information")) {
                const std::string message =
                    curr_robot_builder_env.has_value()
                        ? "- Robot \"" + curr_robot_builder_env.value()->get_robot_name()
                              + "\" selected."
                        : "- No robot selected.";
                ImGui::MenuItem(message.c_str(), nullptr, false, false);

                const std::string member_message =
                    member_focus.has_value() ? "- Member \"" + member_focus.value() + "\" selected."
                                             : "- No member selected.";
                ImGui::MenuItem(member_message.c_str(), nullptr, false, false);

                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem(
                    "Member settings", nullptr, false, curr_robot_builder_env.has_value()))
                show_member_window = true;
            if (ImGui::MenuItem(
                    "Transform member", nullptr, false, curr_robot_builder_env.has_value()))
                show_construct_tools_window = true;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Algorithm")) {
            if (ImGui::BeginMenu("Train")) {
                if (ImGui::MenuItem("training window")) show_training_window = true;
                if (ImGui::MenuItem("on this robot")) { /* TODO run training in new thread */
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Run")) {
                if (ImGui::MenuItem("on this robot")) {
                    /* TODO run trained agent on main display */
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Test / Debug")) { /* TODO run debug agent on robot */
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (!curr_robot_builder_env.has_value()) {
        show_member_window = false;
        show_construct_tools_window = false;
    }
}

void ImGuiApplication::imgui_render_file_dialog() {
    robot_file_dialog.Display();

    if (robot_file_dialog.HasSelected()) {
        std::filesystem::path robot_json_path = robot_file_dialog.GetSelected();

        const auto robot = std::make_shared<RobotBuilderEnvironment>("");
        robot->load_robot(robot_json_path);

        if (!std::any_of(
                opengl_windows.begin(), opengl_windows.end(),
                [robot](const std::shared_ptr<OpenGlWindow> &gl_window) {
                    auto env =
                        std::dynamic_pointer_cast<RobotBuilderEnvironment>(gl_window->get_env());
                    return env && env->get_robot_name() == robot->get_robot_name();
                })) {
            opengl_windows.push_back(std::make_shared<OpenGlWindow>(
                robot->get_robot_name(), robot,
                [this, robot](const glm::vec3 &near, const glm::vec3 &far) {
                    member_focus = robot->ray_cast_member(near, far);
                }));
            curr_robot_builder_env = robot;
            member_focus = std::nullopt;
        } else {
            ImGui::OpenPopup(popup_already_opened_robot.c_str());
        }

        robot_file_dialog.ClearSelected();
    }
}

void ImGuiApplication::imgui_render_construct_tools() {
    if (show_member_window) {
        if (ImGui::Begin("Construct tools", &show_member_window)) {
            std::string message = "No focus member";

            if (curr_robot_builder_env.has_value() && member_focus.has_value()) {
                ImGui::Text("Focus on \"%s\" member", member_focus.value().c_str());

                auto [member_pos, member_rot, member_scale] =
                    curr_robot_builder_env.value()->get_member_transform(member_focus.value());

                ImGui::Separator();
                ImGui::Text("Position");

                bool updated = false;

                if (ImGui::InputFloat("pos.x", &member_pos.x)
                    || ImGui::InputFloat("pos.y", &member_pos.y)
                    || ImGui::InputFloat("pos.z", &member_pos.z))
                    updated = true;

                ImGui::Separator();
                ImGui::Text("Rotation quaternion");

                if (ImGui::InputFloat("quat.w", &member_rot.w)
                    || ImGui::InputFloat("quat.x", &member_rot.w)
                    || ImGui::InputFloat("quat.y", &member_rot.y)
                    || ImGui::InputFloat("quat.z", &member_rot.z))
                    updated = true;

                ImGui::Separator();
                ImGui::Text("Scale");

                if (ImGui::InputFloat("scale.x", &member_scale.x)
                    || ImGui::InputFloat("scale.y", &member_scale.y)
                    || ImGui::InputFloat("scale.z", &member_scale.z))
                    updated = true;

                if (updated)
                    curr_robot_builder_env.value()->update_member(
                        member_focus.value(), member_pos, member_rot, member_scale);

            } else {
                ImGui::Text("No focus member");
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

        std::erase_if(opengl_windows, [this](const auto &gl_window) {
            if (gl_window->draw_imgui_image()) return false;
            curr_robot_builder_env = std::nullopt;
            member_focus = std::nullopt;
            return true;
        });

        ImGui::EndTabBar();
    }

    ImGui::PopStyleVar(3);

    ImGui::End();
}

bool ImGuiApplication::is_close() const { return need_close || glfwWindowShouldClose(window); }

ImGuiApplication::~ImGuiApplication() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}