#pragma once

#include "GL\glew.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "InputHandler.h"
#include "Vector2D.h"

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SENSITIVTY = 0.1f;
const GLfloat FOV = 45.0f;

class Camera
{
public:
	~Camera() {};

	glm::vec3 camera_pos;
	glm::vec3 camera_front;
	glm::vec3 camera_up;
	glm::vec3 camera_right;
	glm::vec3 world_up;

	GLfloat sensitivity;

	GLfloat yaw; // Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
	GLfloat pitch;

	GLfloat fov;

	Camera(
		glm::vec3 pos = glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)
		) : 
		camera_front(glm::vec3(0.0f, 0.0f, -1.0f)),
		yaw(YAW), 
		pitch(PITCH),
		fov(FOV), 
		sensitivity(SENSITIVTY)
	{
		camera_pos = pos;
		world_up = up;

		updateCameraVectors();
	}

	void updateKey(GLfloat delta_time, GLfloat speed)
	{
		GLfloat velocity = speed * delta_time;

		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_LSHIFT))
			velocity *= 2;

		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_W))
			camera_pos += velocity * camera_front;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_S))
			camera_pos -= velocity / 2 * camera_front;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_A))
			camera_pos -= camera_right * velocity;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_D))
			camera_pos += camera_right * velocity;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_SPACE))
			camera_pos += glm::vec3(0.0f, 0.01f * delta_time, 0.0f);
		if(InputHandler::Instance()->isKeyDown(SDL_SCANCODE_LCTRL))
			camera_pos -= glm::vec3(0.0f, 0.01f * delta_time, 0.0f);
	}

	void updateMouse(GLfloat x_offset, GLfloat y_offset)
	{
		x_offset *= sensitivity;
		y_offset *= sensitivity;
		yaw += x_offset;
		pitch -= y_offset;

		if (pitch > 89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;

		glutWarpPointer(m_mousePos.x, m_mousePos.y);

		updateCameraVectors();
	}

	glm::mat4 getViewMatrix()
	{
		return glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
	}

private:
	void updateCameraVectors()
	{
		glm::vec3 forward;

		forward.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		forward.y = sin(glm::radians(pitch));
		forward.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

		camera_front = glm::normalize(forward);

		camera_right = glm::normalize(glm::cross(camera_front, world_up)); // cross product is a vector perpendicular to both vectors
		camera_up = glm::normalize(glm::cross(camera_right, camera_front));
	}
};