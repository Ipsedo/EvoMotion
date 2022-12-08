//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_RENDERER_H
#define EVOMOTION_RENDERER_H

#include <vector>
#include <thread>
#include <glm/glm.hpp>
#include "obj_mtl_vbo.h"
#include <GLFW/glfw3.h>
#include "../model/item.h"

struct renderer {

	float m_width_px, m_height_px;

	GLFWwindow *m_window;

	glm::mat4 m_proj_mat;
	glm::mat4 m_view_mat;
	glm::vec3 m_cam_pos;

	bool m_is_on;

	renderer(int width, int height);

	void init();

	void draw(float delta, std::vector<item> to_draw);
};

#endif //EVOMOTION_RENDERER_H
