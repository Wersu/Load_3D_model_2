#include "Game.h"
#include "InputHandler.h"
#include "GL\glew.h"
#include "GL\freeglut.h"
#include "IL\il.h"
#include "IL\ilu.h"
#include "IL\ilut.h"
#include "SDL\SDL_mixer.h"

#include <iostream>
#include <assert.h>

Game::Game()
{
	screen_width = 1920;
	screen_height = 1080;

	running = true;
}


Game::~Game()
{

}

void Game::init()
{ 
	if (SDL_Init(SDL_INIT_EVERYTHING) >= 0)
	{
		setAttributes();

		setDisplayMode();

		window = SDL_CreateWindow("OpenGL",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,screen_width, screen_height,SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | 0);

 
		if (window != 0)
			glcontext = SDL_GL_CreateContext(window);
	}
	else
	{
		std::cout << "window create error " << std::endl;
		return;  
	}

	SDL_SetWindowFullscreen(window, 0);  

	glewExperimental = GL_TRUE;  
	glewInit();

	ilInit();  
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);
	ilEnable(IL_ORIGIN_SET); 
 	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);  

									 
	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4069);
	Mix_VolumeMusic(1);

	glViewport(0, 0, screen_width, screen_height); 
	glEnable(GL_DEPTH_TEST);  

	int buffer;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &buffer);

	triangle.init();
}

void Game::setAttributes()
{
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
}

void Game::setDisplayMode()
{
	SDL_DisplayMode dMode;
	SDL_GetDesktopDisplayMode(0, &dMode);
	screen_width = dMode.w;
	screen_height = dMode.h;
}

void Game::update()
{
	triangle.update();
}

void Game::render()
{
 	glClear(
		GL_DEPTH_BUFFER_BIT|
		GL_COLOR_BUFFER_BIT|
		GL_STENCIL_BUFFER_BIT
	);

	triangle.render();

	glFlush();  

	SDL_GL_SwapWindow(window); 
}

void Game::handleEvents()
{
	InputHandler::Instance()->updateEvent();
}

void Game::clean()
{
	running = false;

	SDL_DestroyWindow(window);
	SDL_Quit();
}