#pragma once
#include <dinput.h>
#include <Xinput.h>

class InputHandler
{
private:
	IDirectInput8*			m_directInput;

	IDirectInputDevice8*	m_keyboardDevice;
	unsigned char			m_keyboardKeyStates[256];

	IDirectInputDevice8*	m_mouseDevice;
	DIMOUSESTATE			m_mouseStates;

	HWND*					m_hWnd;
	HINSTANCE*				m_hInstance;
	int						m_controllerNum;
	XINPUT_STATE			m_controllerState;
	bool					m_controllerConnected;
public:
	InputHandler(HWND* hWnd, HINSTANCE* hInstance);
	~InputHandler();

	HRESULT InitialiseKeyboardInput();
	
	void ReadInputStates();

	bool IsKeyPressed(unsigned char DI_keycode);

	float GetMouseX(void) { return (float)m_mouseStates.lX; }
	float GetMouseY(void) { return (float)m_mouseStates.lY; }

	bool GetMouseButtonDown(unsigned char mouseKey);
	float GetMouseScroll(void) { return (float)m_mouseStates.lZ; }
};

