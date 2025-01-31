//
// Created by samuel on 09/12/24.
//

#include "./renderer.h"

#include <iostream>

ImGuiRenderer::ImGuiRenderer(const std::string &title, const int width, const int height)
    : need_close(false), clear_color(0.45f, 0.55f, 0.60f, 1.00f), show_member_window(false),
      show_construct_tools_window(false), show_training_window(true),
      curr_robot_builder_env(std::nullopt), opengl_windows(), robot_file_dialog(), rng(1234),
      rd_uni(0.f, 1.f), opengl_render_size(static_cast<float>(width), static_cast<float>(height)), popup_already_opened_robot("Popup_robot_already_opened") {

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

void ImGuiRenderer::draw() {
    glfwPollEvents();

    for (const auto &gl_window: opengl_windows)
        if (gl_window->is_active())
            gl_window->draw_opengl(opengl_render_size.x, opengl_render_size.y);

    std::for_each(opengl_windows.begin(), opengl_windows.end(), [this](const auto &gl_window) {
        if (auto env = std::dynamic_pointer_cast<RobotBuilderEnvironment>(gl_window->get_env());
            gl_window->is_active() && env)
            curr_robot_builder_env = env;
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

    if (ImGui::BeginPopupModal(popup_already_opened_robot.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
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

void ImGuiRenderer::imgui_render_toolbar() {
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Settings")) {}
            if (ImGui::MenuItem("Exit")) need_close = true;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Robots")) {
            std::string message;
            if (curr_robot_builder_env.has_value()) {
                message =
                    "Robot \"" + curr_robot_builder_env.value()->get_robot_name() + "\" selected";
            } else {
                message = "No robot selected";
            }

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 200, 200, 255));
            ImGui::Text("%s", message.c_str());
            ImGui::PopStyleColor();

            ImGui::Separator();

            if (ImGui::MenuItem("New robot")) {

                auto new_robot = std::make_shared<RobotBuilderEnvironment>(
                    "robot_" + std::to_string(opengl_windows.size()));
                opengl_windows.push_back(
                    std::make_shared<OpenGlWindow>(new_robot->get_robot_name(), new_robot));

                curr_robot_builder_env = new_robot;
            }

            if (ImGui::MenuItem("Load robot")) robot_file_dialog.Open();

            if (ImGui::BeginMenu("Show loaded robots")) {
                bool empty = true;
                for (const auto &gl_window: opengl_windows) {
                    if (auto env = std::dynamic_pointer_cast<RobotBuilderEnvironment>(
                            gl_window->get_env());
                        env) {
                        if (ImGui::MenuItem(
                                env->get_robot_name().c_str(), nullptr, gl_window->is_active(), false)) {
                            curr_robot_builder_env = env;
                        }
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

            if (ImGui::BeginMenu("Construct")) {
                if (ImGui::MenuItem("Member settings")) show_member_window = true;
                if (ImGui::MenuItem("Transform member")) show_construct_tools_window = true;
                ImGui::EndMenu();
            }

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
}

void ImGuiRenderer::imgui_render_file_dialog() {
    robot_file_dialog.Display();

    if (robot_file_dialog.HasSelected()) {
        std::filesystem::path robot_json_path = robot_file_dialog.GetSelected();

        const auto robot = std::make_shared<RobotBuilderEnvironment>("");
        robot->load_robot(robot_json_path);

        if (!std::any_of(
                opengl_windows.begin(), opengl_windows.end(),
                [robot](const std::shared_ptr<OpenGlWindow> gl_window) {
                    auto env =
                        std::dynamic_pointer_cast<RobotBuilderEnvironment>(gl_window->get_env());
                    return env && env->get_robot_name() == robot->get_robot_name();
                })) {
            opengl_windows.push_back(
                std::make_shared<OpenGlWindow>(robot->get_robot_name(), robot));
            curr_robot_builder_env = robot;
        } else {
            ImGui::OpenPopup(popup_already_opened_robot.c_str());
        }

        robot_file_dialog.ClearSelected();
    }
}

void ImGuiRenderer::imgui_render_construct_tools() {
    if (show_member_window) {
        if (ImGui::Begin("Construct tools", &show_member_window)) {
            std::string message = "No focus member";

            glm::vec3 pos(0.f);
            glm::quat rot(0.f, glm::vec3(0.f));
            glm::vec3 scale(1.f);

            /*if (member_focus.has_value()) {
                message = "Focus on \"" + member_focus.value() + "\" member";
                const auto [member_pos, member_rot, member_scale] =
                    robot_builder_envs[curr_loaded_robot_index]->get_member_transform(
                        member_focus.value());
                pos = member_pos;
                rot = member_rot;
                scale = member_scale;
            }*/

            ImGui::Text("%s", message.c_str());

            ImGui::Separator();
            ImGui::Text("Position");

            float x_pos = pos.x;
            if (ImGui::InputFloat("x", &x_pos)) { pos.x = x_pos; }
            float y_pos = pos.y;
            if (ImGui::InputFloat("y", &y_pos)) { pos.y = y_pos; }
            float z_pos = pos.z;
            if (ImGui::InputFloat("z", &z_pos)) { pos.z = z_pos; }

            ImGui::Separator();
            ImGui::Text("Rotation");

            ImGui::Separator();
            ImGui::Text("Scale");
        }
        ImGui::End();
    }
}

void ImGuiRenderer::imgui_render_opengl() {
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

        std::erase_if(opengl_windows, [this](const auto gl_window) {
            if (gl_window->draw_imgui_image()) return false;
            curr_robot_builder_env = std::nullopt;
            return true;
        });

        ImGui::EndTabBar();
    }

    ImGui::PopStyleVar(3);

    ImGui::End();
}

bool ImGuiRenderer::is_close() const { return need_close || glfwWindowShouldClose(window); }

ImGuiRenderer::~ImGuiRenderer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}