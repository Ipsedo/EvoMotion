//
// Created by samuel on 09/12/24.
//

#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <evo_motion_view/renderer.h>

ImGuiRenderer::ImGuiRenderer(const std::string &title, const int width, const int height)
    : show_menu(true), need_close(false) {

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
}

void ImGuiRenderer::draw() {
    constexpr auto clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Demo
    if (show_menu) ImGui::ShowDemoWindow(&show_menu);

    // MenuBar
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("Open")) { /* TODO open robot JSON */
            }
            if (ImGui::MenuItem("Save")) {
                /* TODO save robot JSON on the same file name or create new one */
            }
            if (ImGui::MenuItem("Save As")) { /* TODO save robot JSON to a new file */
            }
            if (ImGui::MenuItem("Exit")) need_close = true;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {

            if (ImGui::MenuItem("Open Menu")) show_menu = true;
            if (ImGui::MenuItem("Construct Menu")) {
                /* TODO display the panel like in GIMP to construct the robot */
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Algorithm")) {

            if (ImGui::BeginMenu("Train")) {
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

bool ImGuiRenderer::is_close() const { return need_close || glfwWindowShouldClose(window); }

ImGuiRenderer::~ImGuiRenderer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}