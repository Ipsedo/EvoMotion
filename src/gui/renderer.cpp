//
// Created by samuel on 09/12/24.
//

#include "./renderer.h"

#include <iostream>

ImGuiRenderer::ImGuiRenderer(const std::string &title, const int width, const int height)
    : need_close(false), show_construct_tools(false), show_training(true),
      show_construct_robot(false), loaded_robots(), curr_loaded_robot_index(-1),
      robot_file_dialog(), drawables(), rng(1234), rd_uni(0.f, 1.f),
      opengl_render_size(width, height) {

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

    // MenuBar
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
                init_drawables();
                show_construct_robot = true;
            }

            if (ImGui::MenuItem("Load robot")) robot_file_dialog.Open();

            if (ImGui::BeginMenu("Select robot")) {
                for (int i = 0; i < loaded_robots.size(); i++) {
                    if (ImGui::MenuItem(
                            loaded_robots[i]->get_robot_name().c_str(), nullptr,
                            curr_loaded_robot_index == i)) {
                        curr_loaded_robot_index = i;
                        init_drawables();
                        show_construct_robot = true;
                    }
                }

                if (loaded_robots.size() == 0) {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 200, 200, 255));
                    ImGui::Text("No robot(s) loaded");
                    ImGui::PopStyleColor();
                }

                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Construct tools")) show_construct_tools = true;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Algorithm")) {
            if (ImGui::BeginMenu("Train")) {
                if (ImGui::MenuItem("training window")) show_training = true;
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

    if (loaded_robots.size() > 0 && show_construct_robot) {
        std::string robot_construct_message =
            "Construct robot \"" + loaded_robots[curr_loaded_robot_index]->get_robot_name() + "\"";
        if (ImGui::Begin(robot_construct_message.c_str(), &show_construct_robot)) {
            opengl_render_size = ImGui::GetContentRegionAvail();
            ImGui::Image(
                frame_buffer->get_frame_texture(), opengl_render_size, ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::End();
    }

    robot_file_dialog.Display();

    if (robot_file_dialog.HasSelected()) {
        std::filesystem::path robot_json_path = robot_file_dialog.GetSelected();

        const auto robot = std::make_shared<RobotBuilderEnvironment>("");
        robot->load_robot(robot_json_path);

        loaded_robots.push_back(robot);
        curr_loaded_robot_index = loaded_robots.size() - 1;
        init_drawables();
        show_construct_robot = true;

        robot_file_dialog.ClearSelected();
    }

    // Demo
    //if (show_menu) ImGui::ShowDemoWindow(&show_menu);

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

void ImGuiRenderer::init_drawables() {
    drawables.clear();

    for (const auto &i: loaded_robots[curr_loaded_robot_index]->get_items())
        drawables[i.get_name()] = std::make_shared<ObjSpecularFactory>(
                                      i.get_shape()->get_vertices(), i.get_shape()->get_normals(),
                                      glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f),
                                      glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f),
                                      glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f), 300.f)
                                      ->get_drawable();

    std::cout << drawables.size() << std::endl;
}

void ImGuiRenderer::render_opengl_robot() {
    frame_buffer->rescale_frame_buffer(opengl_render_size.x, opengl_render_size.y);

    frame_buffer->bind();

    glClearColor(0.5f, 0.1f, 0.1f, 1.f);
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