#include "MScene.h"
#include "Game.h"
#include "SkyBox.h"
#include "TextRendering.h"
#include "ForShader.h"
#include "BillboardList.h"
#include "IL\il.h"
#include "IL\ilu.h"
#include "IL\ilut.h"

Scene::Scene()
{
	glm::vec3 position(2.0f, 5.0f, 18.0f);
	camera = Camera(position);

	mouse_first_in = true;
}

Scene::~Scene()
{
	glDeleteProgram(shaders_animated_model);
}

void Scene::init()
{
	shaders_animated_model = ForShader::createProgram("shaders/animated_model.vert", "shaders/animated_model.frag");

	model_human.setShader(shaders_animated_model);
	model_human.setModel("models/dwarf/Dwarf Walk.dae");
	matr_model = glm::rotate(matr_model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	matr_model = glm::translate(matr_model, glm::vec3(0.0f, 0.0f, -1.0f));
	matr_model = glm::scale(matr_model, glm::vec3(0.02f, 0.02f, 0.02f));

	SkyBox::Instance()->init("images/skybox");

	text_matrix_2D = glm::ortho(0.0f, (float)Game::Instance()->screen_width, 0.0f, (float)Game::Instance()->screen_height, 1.0f, -1.0f);
	
	music1 = Mix_LoadMUS("music/clip.mp3");

	Mix_VolumeMusic(10);
	Mix_PlayMusic(music1, -1); 
}

void Scene::update()
{
	GLfloat current_frame = SDL_GetTicks();
	delta_time = (current_frame - last_frame);
	last_frame = current_frame;

	camera.updateKey(delta_time, speed);

	if (InputHandler::Instance()->getMouseButtonState(LEFT_PRESSED) )
	{
		mouse_position = InputHandler::Instance()->getMousePosition();

		if (mouse_first_in) 
		{
			last_x = mouse_position.getX();
			last_y = mouse_position.getY();
			mouse_first_in = false;
		}

		GLfloat x_offset = mouse_position.getX() - last_x;
		GLfloat y_offset = mouse_position.getY() - last_y;

		last_x = mouse_position.getX();
		last_y = mouse_position.getY();

		camera.updateMouse(x_offset, y_offset);
	}

	if (InputHandler::Instance()->getMouseButtonState(LEFT_RELEASED))
		mouse_first_in = true;

	perspective_view = camera.getViewMatrix();
	perspective_projection = glm::perspective(glm::radians(camera.fov), (float)Game::Instance()->screen_width / (float)Game::Instance()->screen_height, 1.0f, 2000.0f); // ïèðàìèäà

	model_human.update();
	
	glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(0.02f, 0.02f, 0.0f));
	glm::mat4 set_text_to_origin = glm::translate(glm::mat4(), glm::vec3(-1.8f, -0.4f, 0.0f));
	glm::mat4 text_rotate_y = glm::rotate(glm::mat4(), glm::radians(-camera.yaw - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 text_rotate_x = glm::rotate(glm::mat4(), glm::radians(camera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	
	SkyBox::Instance()->update(perspective_projection * glm::mat4(glm::mat3(perspective_view)));
}

void Scene::render()
{
	glUseProgram(shaders_animated_model);

	glUniform3f(glGetUniformLocation(shaders_animated_model, "view_pos"), camera.camera_pos.x, camera.camera_pos.y, camera.camera_pos.z);
	glUniform1f(glGetUniformLocation(shaders_animated_model, "material.shininess"), 32.0f);
	glUniform1f(glGetUniformLocation(shaders_animated_model, "material.transparency"), 1.0f);
	
	glUniform3f(glGetUniformLocation(shaders_animated_model, "point_light.position"), camera.camera_pos.x, camera.camera_pos.y, camera.camera_pos.z);

	glUniform3f(glGetUniformLocation(shaders_animated_model, "point_light.ambient"), 0.1f, 0.1f, 0.1f);
	glUniform3f(glGetUniformLocation(shaders_animated_model, "point_light.diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shaders_animated_model, "point_light.specular"), 1.0f, 1.0f, 1.0f);

	glUniform1f(glGetUniformLocation(shaders_animated_model, "point_light.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shaders_animated_model, "point_light.linear"), 0.007);	
	glUniform1f(glGetUniformLocation(shaders_animated_model, "point_light.quadratic"), 0.0002);

	MVP = perspective_projection * perspective_view * matr_model;//позиция объекта
	glUniformMatrix4fv(glGetUniformLocation(shaders_animated_model, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glUniformMatrix4fv(glGetUniformLocation(shaders_animated_model, "M_matrix"), 1, GL_FALSE, glm::value_ptr(matr_model));
	glm::mat4 matr_normals_cube = glm::mat4(glm::transpose(glm::inverse(matr_model)));
	glUniformMatrix4fv(glGetUniformLocation(shaders_animated_model, "normals_matrix"), 1, GL_FALSE, glm::value_ptr(matr_normals_cube));
	model_human.draw(shaders_animated_model);

	glUseProgram(0);

	SkyBox::Instance()->render();

	glDepthFunc(GL_ALWAYS);
	
	glDepthFunc(GL_LESS);
}

GLuint Scene::loadImageToTexture(const char* image_path)
{
	ILuint ImageIndex; 

	ilGenImages(1, &ImageIndex); 
	ilBindImage(ImageIndex); 

	if (!ilLoadImage((ILstring)image_path)) 
		std::cout << "image NOT load " << std::endl;

	GLuint textureID;
	glGenTextures(1, &textureID); 
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	ilDeleteImages(1, &ImageIndex);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

GLuint Scene::loadDDS(const char* image_path, int *w, int *h)//загружаем картинку с расширением ддс
{
	unsigned char header[124];

	FILE *fp;
	fp = fopen(image_path, "rb");
	if (fp == NULL)
	{
		std::cout << "image not load \n";
		return 0;
	}

	char filecode[4];
	fread(filecode, 1, 4, fp);
	if (strncmp(filecode, "DDS ", 4) != 0) {
		std::cout << "return 0; \n";
		fclose(fp);
		return 0;
	}

	fread(&header, 124, 1, fp);

	unsigned int height = *(unsigned int*)&(header[8]);
	unsigned int width = *(unsigned int*)&(header[12]);
	unsigned int linear_size = *(unsigned int*)&(header[16]);
	unsigned int mipmap_count = *(unsigned int*)&(header[24]);
	unsigned int four_cc = *(unsigned int*)&(header[80]);  

	if (w != nullptr) 
		*w = width;
	if (h != nullptr) 
		*h = height;

	unsigned char *buffer;
	unsigned int buff_size = mipmap_count > 1 ? linear_size * 2 : linear_size;
	buffer = (unsigned char*)malloc(buff_size * sizeof(unsigned char));
	fread(buffer, 1, buff_size, fp);

	fclose(fp);

#define FOURCC_DXT1 0x31545844 
#define FOURCC_DXT3 0x33545844 
#define FOURCC_DXT5 0x35545844 

	unsigned int components = (four_cc == FOURCC_DXT1) ? 3 : 4;;
	unsigned int format;
	if (four_cc == FOURCC_DXT1)
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	else if (four_cc == FOURCC_DXT3)
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	else if (four_cc == FOURCC_DXT5)
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

#undef FOURCC_DXT1
#undef FOURCC_DXT3
#undef FOURCC_DXT5

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (mipmap_count == 1) 
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else 
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;;
	unsigned int offset = 0;
	
	for (unsigned int level = 0; level < mipmap_count && (width || height); level++)
	{
		unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset);

		offset += size;
		width /= 2;
		height /= 2;
	}

	free(buffer);

	return textureID;
}
