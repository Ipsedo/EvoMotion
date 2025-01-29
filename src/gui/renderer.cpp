//
// Created by samuel on 09/12/24.
//

#include "./renderer.h"

#include <iostream>

ImGuiRenderer::ImGuiRenderer(const std::string &title, const int width, const int height)
    : need_close(false), show_member_window(false), show_construct_tools_window(false),
      show_training_window(true), show_robot_builder_window(false), loaded_robots(),
      curr_loaded_robot_index(-1), member_focus(std::nullopt), robot_file_dialog(), drawables(),
      rng(1234), rd_uni(0.f, 1.f), opengl_render_size(width, height) {

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
    constexpr auto clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glfwPollEvents();

    if (curr_loaded_robot_index >= 0) render_opengl_robot();

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
    imgui_render_robot_construct();
    imgui_render_construct_tools();
    imgui_render_file_dialog();

    // End - render
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(
        clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
        clear_color.w);
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
            if (curr_loaded_robot_index >= 0) {
                message = "Robot \"" + loaded_robots[curr_loaded_robot_index]->get_robot_name()
                          + "\" selected";
            } else {
                message = "No robot selected";
            }

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 200, 200, 255));
            ImGui::Text("%s", message.c_str());
            ImGui::PopStyleColor();

            ImGui::Separator();

            if (ImGui::MenuItem("New robot")) {
                auto new_robot = std::make_shared<RobotBuilderEnvironment>(
                    "robot_" + std::to_string(loaded_robots.size()));
                loaded_robots.push_back(new_robot);
                curr_loaded_robot_index = loaded_robots.size() - 1;
                init_robot();
                show_robot_builder_window = true;
            }

            if (ImGui::MenuItem("Load robot")) robot_file_dialog.Open();

            if (ImGui::BeginMenu("Select robot")) {
                for (int i = 0; i < loaded_robots.size(); i++) {
                    if (ImGui::MenuItem(
                            loaded_robots[i]->get_robot_name().c_str(), nullptr,
                            curr_loaded_robot_index == i)) {
                        curr_loaded_robot_index = i;
                        init_robot();
                        show_robot_builder_window = true;
                    }
                }

                if (loaded_robots.size() == 0) {
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

            if (ImGui::MenuItem("Close robot")) {
                show_robot_builder_window = false;
                curr_loaded_robot_index = -1;
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

        loaded_robots.push_back(robot);
        curr_loaded_robot_index = loaded_robots.size() - 1;
        init_robot();
        show_robot_builder_window = true;

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

            if (member_focus.has_value()) {
                message = "Focus on \"" + member_focus.value() + "\" member";
                const auto [member_pos, member_rot, member_scale] =
                    loaded_robots[curr_loaded_robot_index]->get_member_transform(
                        member_focus.value());
                pos = member_pos;
                rot = member_rot;
                scale = member_scale;
            }

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

void ImGuiRenderer::imgui_render_robot_construct() {
    if (loaded_robots.size() > 0 && show_robot_builder_window) {
        std::string robot_construct_message =
            "Construct robot \"" + loaded_robots[curr_loaded_robot_index]->get_robot_name() + "\"";

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImVec2 window_pos = viewport->Pos;
        ImVec2 available_size = viewport->Size;

        float menu_bar_height = ImGui::GetFrameHeight();
        window_pos.y += menu_bar_height;
        available_size.y -= menu_bar_height;

        // Create the full-space background window
        ImGui::SetNextWindowPos(window_pos);
        ImGui::SetNextWindowSize(available_size);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        if (ImGui::Begin(robot_construct_message.c_str(), &show_robot_builder_window, flags)) {
            opengl_render_size = ImGui::GetContentRegionAvail();
            ImGui::Image(
                frame_buffer->get_frame_texture(), opengl_render_size, ImVec2(0, 1), ImVec2(1, 0));
        }

        ImGui::PopStyleVar(3);

        ImGui::End();
    }
}

void ImGuiRenderer::init_robot() {
    drawables.clear();

    for (const auto &i: loaded_robots[curr_loaded_robot_index]->get_items())
        drawables[i.get_name()] = std::make_shared<ObjSpecularFactory>(
                                      i.get_shape()->get_vertices(), i.get_shape()->get_normals(),
                                      glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f),
                                      glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f),
                                      glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f), 300.f)
                                      ->get_drawable();

    member_focus = std::nullopt;
}

void ImGuiRenderer::render_opengl_robot() {
    frame_buffer->rescale_frame_buffer(opengl_render_size.x, opengl_render_size.y);

    frame_buffer->bind();

    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glViewport(0, 0, opengl_render_size.x, opengl_render_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::vec3 camera_pos(0, 1, 4);
    const auto view_matrix = glm::lookAt(camera_pos, glm::vec3(0), glm::vec3(0, 1, 0));
    const auto projection_matrix = glm::frustum(
        -1.f, 1.f,
        -static_cast<float>(opengl_render_size.y) / static_cast<float>(opengl_render_size.x),
        static_cast<float>(opengl_render_size.y) / static_cast<float>(opengl_render_size.x), 1.f,
        200.f);

    for (const auto &i: loaded_robots[curr_loaded_robot_index]->get_items())
        drawables[i.get_name()]->draw(
            projection_matrix, view_matrix, i.model_matrix(), glm::vec3(0, 20, 0), camera_pos);

    frame_buffer->unbind();
}

bool ImGuiRenderer::is_close() const { return need_close || glfwWindowShouldClose(window); }

ImGuiRenderer::~ImGuiRenderer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}