//
// Created by samuel on 11/08/19.
//

#include "renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <SOIL/SOIL.h>
#include "error.h"
#include "../utils/res.h"


renderer::renderer(int width, int height) :
		m_width_px(width), m_height_px(height), m_is_on(false) {}

void renderer::draw(float delta, std::vector<item> to_draw) {
	// Init ObjMtlVBO if not yet
	for (item t : to_draw)
		if (!t.m_obj_mtl_vbo->can_draw())
			t.m_obj_mtl_vbo->init();

	// Kill program if OpenGL & GLFW not initialized
	if (!m_is_on) {
		std::cout << "OpenGL and GLFW context not initialized !" << std::endl;
		exit(1);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (item t : to_draw) {
		btScalar tmp[16];
		btTransform tr;
		t.m_rg_body->getMotionState()->getWorldTransform(tr);
		tr.getOpenGLMatrix(tmp);

		glm::mat4 model_mat = glm::make_mat4(tmp) * glm::scale(glm::mat4(1.f), t.m_obj_mtl_vbo_scale);

		glm::mat4 mv_mat = m_view_mat * model_mat;
		glm::mat4 mvp_mat = m_proj_mat * mv_mat;

		t.m_obj_mtl_vbo->draw(mvp_mat, mv_mat, glm::vec3(0.f), m_cam_pos);
	}

	glfwSwapBuffers(m_window);

	glfwPollEvents();

	bool will_draw_next_frame = !glfwWindowShouldClose(m_window);

	// If will quit renderer
	if (!will_draw_next_frame) {
		// Stop GLFW & OpenGL context
		glfwDestroyWindow(m_window);
		glfwTerminate();

		m_is_on = false;

		// Stop ObjMtlVBO in prevent of new OpenGL context
		// (will be reinitialized)
		for (item t : to_draw)
			t.m_obj_mtl_vbo->kill();
	}
}


void renderer::init() {
	if (!glfwInit()) {
		fprintf(stderr, "Failed GLFW initialization\n");
		exit(0);
	}

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_SAMPLES, 4);
	m_window = glfwCreateWindow(int(m_width_px), int(m_height_px), "EvoMotion", NULL, NULL);
	if (!m_window) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		exit(0);
	}

	glfwMakeContextCurrent(m_window);
	glfwSetErrorCallback(error_callback);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		exit(0);
	}

	GLFWimage icons[1];
	icons[0].pixels = SOIL_load_image((get_res_folder() + EVOMOTION_SEP + "icon.png").c_str(),
			&icons[0].width, &icons[0].height, 0, SOIL_LOAD_RGBA);
	glfwSetWindowIcon(m_window, 1, icons);

	m_cam_pos = glm::vec3(0., 0., -1.);
	m_proj_mat = glm::frustum(-1.f, 1.f, -m_height_px / m_width_px, m_height_px / m_width_px, 1.0f, 200.0f);
	m_view_mat = glm::lookAt(
			glm::vec3(0., 0., 0.),
			glm::vec3(0., 0., 1.),
			glm::vec3(0, 1, 0)
	);

	glViewport(0, 0, int(m_width_px), int(m_height_px));
	glClearColor(0.5, 0.0, 0.0, 1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	m_is_on = true;
}
