#include "Game.h"
#include <SDL\SDL.h>
#include <iostream>
#include <string>

const int FPS = 60;
const int LOOP_TIME = 1000 / FPS;  

 

int main(int argc, char* argv[])
{
	Uint32 frameStart, frameTime;

	Game::Instance()->init();

	while (Game::Instance()->isRunning())
	{
		frameStart = SDL_GetTicks();

		Game::Instance()->update();
		Game::Instance()->render();
		Game::Instance()->handleEvents();
 
		frameTime = SDL_GetTicks() - frameStart;  

		if (frameTime < LOOP_TIME)
			SDL_Delay((int)(LOOP_TIME - frameTime));
	}

	Game::Instance()->clean();
 
	return 0;
}