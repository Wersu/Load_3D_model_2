#pragma once

#include "GL\glew.h"
#include "SDL\SDL_opengl.h"

#include <iostream>

class ForShader
{
public:
	ForShader();
	~ForShader();

	static const GLchar* readShader(const char* filename);

	static GLuint createProgram(const char* vertex, const char* fragment);
};

