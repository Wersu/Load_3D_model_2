#pragma once

#include "GL\glew.h"
#include <iostream>
#include "Camera.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SDL\SDL_mixer.h"

class Scene
{
public:
	Scene();
	~Scene();

	GLfloat speed = 0.02f;
	//for camera
	GLfloat delta_time = 0.0f;
	GLfloat last_frame = 0.0f;
	GLfloat last_x;
	GLfloat last_y;
	Vector2D mouse_position;
	GLboolean mouse_first_in;
	Camera camera;

	GLuint shaders_animated_model;
	Model model_human;

	// music
	Mix_Music* music1 = nullptr;

	glm::mat4 MVP;
	glm::mat4 perspective_view;
	glm::mat4 perspective_projection;
	glm::mat4 matr_model;

	// text
	glm::mat4 text_matrix_2D; // orthographic projection
	glm::mat4 text_matrix_3D_model_1; // perspective  projection

	void init();
	void update();
	void render();

	static GLuint loadDDS(const char* image_path, int* w = nullptr, int* h = nullptr);
	static GLuint loadImageToTexture(const char* image_path);
};

