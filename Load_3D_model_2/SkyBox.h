#pragma once

#include "GL\glew.h"
#include "glm\glm.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "Camera.h"

using namespace std;

class SkyBox
{
public:
	static SkyBox* Instance()
	{
		static SkyBox inst;
		return &inst;
	}

	void init(char* folder);
	void update(glm::mat4 VP_matr);
	void render();

private:
	SkyBox();
	~SkyBox();

	GLuint VAO_skybox;
	GLuint VBO_vertices_textures;

	GLuint texture_id;
	GLuint createTexture(char* folder);

	GLuint skybox_shaders;

	glm::mat4 VP_matrix; // WITHOUT model for skybox
};

