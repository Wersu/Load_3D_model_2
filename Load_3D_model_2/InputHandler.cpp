#include "InputHandler.h"
#include "Game.h"


InputHandler::InputHandler()
{
	m_mouseButtonState.reserve(6);

	for (int i = 0; i < 6; i++)
		m_mouseButtonState.push_back(false);
}


void InputHandler::updateEvent()
{
	SDL_Event inputEvent;
	while (SDL_PollEvent(&inputEvent))
	{
		h_keyStates = SDL_GetKeyboardState(0);

		switch (inputEvent.type)
		{
		case SDL_QUIT:
			Game::Instance()->clean();
			break;

		case SDL_MOUSEBUTTONDOWN:
			onMouseButtonDown(inputEvent);
			break;

		case SDL_MOUSEBUTTONUP:
			onMouseButtonUp(inputEvent);
			break;

		case SDL_MOUSEMOTION:
			onMouseMove(inputEvent);
			break;
		}
	}
}

void InputHandler::onMouseMove(SDL_Event& inputEvent)
{
	m_mousePosition.setX(inputEvent.motion.x);
	m_mousePosition.setY(inputEvent.motion.y);
}

void InputHandler::onMouseButtonDown(SDL_Event& inputEvent)
{
	if (inputEvent.button.button == SDL_BUTTON_LEFT)
	{
		m_mouseButtonState[LEFT_PRESSED] = true;
		m_mouseButtonState[LEFT_RELEASED] = false;
	}
	if (inputEvent.button.button == SDL_BUTTON_MIDDLE)
	{
		m_mouseButtonState[MIDDLE_PRESSED] = true;
		m_mouseButtonState[MIDDLE_RELEASED] = false;
	}
	if (inputEvent.button.button == SDL_BUTTON_RIGHT)
	{
		m_mouseButtonState[RIGHT_PRESSED] = true;
		m_mouseButtonState[RIGHT_RELEASED] = false;
	}
}

void InputHandler::onMouseButtonUp(SDL_Event& inputEvent)
{
	if (inputEvent.button.button == SDL_BUTTON_LEFT)
	{
		m_mouseButtonState[LEFT_RELEASED] = true;
		m_mouseButtonState[LEFT_PRESSED] = false;
	}

	if (inputEvent.button.button == SDL_BUTTON_MIDDLE)
	{
		m_mouseButtonState[MIDDLE_RELEASED] = true;
		m_mouseButtonState[MIDDLE_PRESSED] = false;
	}

	if (inputEvent.button.button == SDL_BUTTON_RIGHT)
	{
		m_mouseButtonState[RIGHT_RELEASED] = true;
		m_mouseButtonState[RIGHT_PRESSED] = false;
	}
}

bool InputHandler::isKeyDown(SDL_Scancode key)
{
	if (h_keyStates != 0)
		if (h_keyStates[key] == 1)
			return true;

	return false;
}

bool InputHandler::isKeyNotDown(SDL_Scancode key)
{
	if (h_keyStates != 0)
		if (h_keyStates[key] == 1)
			return true;

	return false;
}