#include "InputHandler.h"

InputHandler::InputHandler(HWND* hWnd, HINSTANCE* hInstance)
{
	m_hInstance = hInstance;
	m_hWnd = hWnd;
	m_controllerConnected = false;
	m_controllerNum = 0;
}

//Clean up
InputHandler::~InputHandler()
{
	if (m_mouseDevice)
	{
		m_mouseDevice->Unacquire();
		m_mouseDevice->Release();
	}
	if (m_keyboardDevice)
	{
		m_keyboardDevice->Unacquire();
		m_keyboardDevice->Release();
	}
	if (m_directInput) m_directInput->Release();
	if (m_hWnd) m_hWnd = nullptr;
	if (m_hInstance) m_hInstance = nullptr;
}

//Set up the inputs
HRESULT InputHandler::InitialiseKeyboardInput()
{
	HRESULT hr;

	ZeroMemory(m_keyboardKeyStates, sizeof(m_keyboardKeyStates));

	hr = DirectInput8Create(*m_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);

	if (FAILED(hr))	return hr;

	hr = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboardDevice, NULL);

	if (FAILED(hr))	return hr;

	hr = m_keyboardDevice->SetDataFormat(&c_dfDIKeyboard);

	if (FAILED(hr))	return hr;
	

	hr = m_keyboardDevice->SetCooperativeLevel(*m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	if (FAILED(hr)) return hr;


	hr = m_directInput->CreateDevice(GUID_SysMouse, &m_mouseDevice, NULL);

	if (FAILED(hr)) return hr;

	hr = m_mouseDevice->SetDataFormat(&c_dfDIMouse);
	
	if (FAILED(hr)) return hr;

	hr = m_mouseDevice->SetCooperativeLevel(*m_hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);

	if (FAILED(hr)) return hr;

	return S_OK;
}

void InputHandler::ReadInputStates()
{
	HRESULT hr;
	hr = m_keyboardDevice->GetDeviceState(sizeof(m_keyboardKeyStates), (LPVOID)&m_keyboardKeyStates);

	if (FAILED(hr))
	{
		//Reacquire the keyboard if it doesnt exist
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			m_keyboardDevice->Acquire();
		}
	}
	
	hr = m_mouseDevice->GetDeviceState(sizeof(m_mouseStates), (LPVOID)&m_mouseStates);
	if (FAILED(hr))
	{
		//Reacquire the mouse if it doesnt exist
		if ((hr == DIERR_NOTACQUIRED) || (hr == DIERR_INPUTLOST))
		{
			m_mouseDevice->Acquire();
		}
	}
}

//bool returns whether the inputted key is press
bool InputHandler::IsKeyPressed(unsigned char DI_keycode)
{
	return m_keyboardKeyStates[DI_keycode] & 0x80;
}

//Returns whether the inputted mouse button is pressed
bool InputHandler::GetMouseButtonDown(unsigned char mouseKey)
{
	return m_mouseStates.rgbButtons[mouseKey] & 0x80;
}