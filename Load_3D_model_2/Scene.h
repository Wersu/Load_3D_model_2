#pragma once

#include "GL\glew.h"
#include <iostream>
#include "Camera.h"
#include "Model.h"


class Scene
{
public:
	GLfloat speed = 0.02f;
	//for camera
	GLfloat delta_time = 0.0f;
	GLfloat last_frame = 0.0f;
	GLfloat last_x;
	GLfloat last_y;
	Vector2D mouse_position;
	GLboolean mouse_first_in;
	Camera camera
};

