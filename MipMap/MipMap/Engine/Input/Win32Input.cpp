#include "Win32Input.h"
#include <string> 

#define KEY_PRESSED 0x8000 

namespace Input
{
	InputManager::InputManager(const HWND* windowHandle)
		: windowHandle(windowHandle)
	{
		keysRegistered = (InputFlags::KeyFlags)0;
		keysHeld = (InputFlags::KeyFlags)0;
		keysPressed = (InputFlags::KeyFlags)0;
		keysReleased = (InputFlags::KeyFlags)0;
		SetCursorPos(5, 5);

		// mouseTracker = (MouseTracker)(MouseTracker::WindowSelected | MouseTracker::OverWindows);
	}

	InputManager::~InputManager()
	{
	}
	

	void InputManager::Update()
	{
		UpdateRegisteredKeys();
		UpdateMouse();
	}

	void InputManager::RegisterKeys(InputFlags::KeyFlags keys)
	{
		keysRegistered = (InputFlags::KeyFlags)(keysRegistered | keys);
	}

	void InputManager::RegisterKey(Key key)
	{
		keysRegistered = (InputFlags::KeyFlags)(keysRegistered | (InputFlags::KeyFlags)(1 << (int)key));
	}

	void InputManager::UnregisterKeys(InputFlags::KeyFlags keys)
	{
		keysRegistered = (InputFlags::KeyFlags)(keysRegistered & ~keys);
	}

	void InputManager::UnregisterKey(Key key)
	{
		keysRegistered = (InputFlags::KeyFlags)(keysRegistered & ~(InputFlags::KeyFlags)(1 << (int)key));
	}

	InputFlags::KeyFlags InputManager::GetRegisteredKeys() const
	{
		return keysRegistered;
	}

	bool InputManager::IsKeyRegistered(Key key) const
	{
		return keysRegistered & (InputFlags::KeyFlags)(1 << (int)key);
	}

	KeyState InputManager::GetRegisteredKeyState(Key key) const
	{
		if (!(keysRegistered & (InputFlags::KeyFlags)(1 << (int)key)))
			return NotRegistered;

		if (keysHeld & (InputFlags::KeyFlags)(1 << (int)key))
			return Held;
		if (keysPressed & (InputFlags::KeyFlags)(1 << (int)key))
			return Pressed;
		if (keysReleased& (InputFlags::KeyFlags)(1 << (int)key))
			return Released;

		return NotPressed;
	}

	KeyState InputManager::GetMouseButtonState(MouseButton mouseButton) const
	{
		return KeyState();
	}

	void InputManager::SetMouseTracker(MouseTracker mouseTracker)
	{
		this->mouseTracker = mouseTracker;
	}

	const POINT* InputManager::GetMousePositionPointer() const
	{
		return &mousePosition;
	}

	const POINT* InputManager::GetMousePositionDeltaPointer() const
	{
		return &mouseDelta;
	}

	void InputManager::UpdateRegisteredKeys()
	{
		for (short currentKeyIndex = 0; currentKeyIndex < 64; currentKeyIndex++)
		{
			if (!(keysRegistered & ((long long)1 << currentKeyIndex)))
			{
				// renderer->DrawString(1, 4, std::to_string(currentKeyIndex));
				// key is not registered. Do not look
				continue;
			}
			Key key = (Key)currentKeyIndex;

			switch (key)
			{
			case A:
				ReadKeyValue('A', currentKeyIndex);
				break;
			case B:
				ReadKeyValue('B', currentKeyIndex);
				break;
			case C:
				ReadKeyValue('C', currentKeyIndex);
				break;
			case D:
				ReadKeyValue('D', currentKeyIndex);
				break;
			case E:
				ReadKeyValue('E', currentKeyIndex);
				break;
			case F:
				ReadKeyValue('F', currentKeyIndex);
				break;
			case G:
				ReadKeyValue('G', currentKeyIndex);
				break;
			case H:
				ReadKeyValue('H', currentKeyIndex);
				break;
			case I:
				ReadKeyValue('I', currentKeyIndex);
				break;
			case J:
				ReadKeyValue('J', currentKeyIndex);
				break;
			case K:
				ReadKeyValue('K', currentKeyIndex);
				break;
			case L:
				ReadKeyValue('L', currentKeyIndex);
				break;
			case M:
				ReadKeyValue('M', currentKeyIndex);
				break;
			case N:
				ReadKeyValue('N', currentKeyIndex);
				break;
			case O:
				ReadKeyValue('O', currentKeyIndex);
				break;
			case P:
				ReadKeyValue('P', currentKeyIndex);
				break;
			case Q:
				ReadKeyValue('Q', currentKeyIndex);
				break;
			case R:
				ReadKeyValue('R', currentKeyIndex);
				break;
			case S:
				ReadKeyValue('S', currentKeyIndex);
				break;
			case T:
				ReadKeyValue('T', currentKeyIndex);
				break;
			case U:
				ReadKeyValue('U', currentKeyIndex);
				break;
			case V:
				ReadKeyValue('V', currentKeyIndex);
				break;
			case W:
				ReadKeyValue('W', currentKeyIndex);
				break;
			case X:
				ReadKeyValue('X', currentKeyIndex);
				break;
			case Y:
				ReadKeyValue('Y', currentKeyIndex);
				break;
			case Z:
				ReadKeyValue('Z', currentKeyIndex);
				break;
			case One:
				ReadKeyValue('1', currentKeyIndex);
				break;
			case Two:
				ReadKeyValue('2', currentKeyIndex);
				break;
			case Three:
				ReadKeyValue('3', currentKeyIndex);
				break;
			case Four:
				ReadKeyValue('4', currentKeyIndex);
				break;
			case Five:
				ReadKeyValue('5', currentKeyIndex);
				break;
			case Six:
				ReadKeyValue('6', currentKeyIndex);
				break;
			case Seven:
				ReadKeyValue('7', currentKeyIndex);
				break;
			case Eight:
				ReadKeyValue('8', currentKeyIndex);
				break;
			case Nine:
				ReadKeyValue('9', currentKeyIndex);
				break;
			case Zero:
				ReadKeyValue('0', currentKeyIndex);
				break;
			case Ctrl:
				ReadKeyValue(VK_CONTROL, currentKeyIndex);
				break;
			case LCtrl:
				ReadKeyValue(VK_LCONTROL, currentKeyIndex);
				break;
			case RCtrl:
				ReadKeyValue(VK_RCONTROL, currentKeyIndex);
				break;
			case Shift:
				ReadKeyValue(VK_SHIFT, currentKeyIndex);
				break;
			case LShift:
				ReadKeyValue(VK_LSHIFT, currentKeyIndex);
				break;
			case RShift:
				ReadKeyValue(VK_RSHIFT, currentKeyIndex);
				break;
			case Alt:
				ReadKeyValue(VK_MENU, currentKeyIndex);
				break;
			case LAlt:
				ReadKeyValue(VK_LMENU, currentKeyIndex);
				break;
			case RAlt:
				ReadKeyValue(VK_RMENU, currentKeyIndex);
				break;
			case Esc:
				ReadKeyValue(VK_ESCAPE, currentKeyIndex);
				break;
			case Tab:
				ReadKeyValue(VK_TAB, currentKeyIndex);
				break;
			case Enter:
				ReadKeyValue(VK_RETURN, currentKeyIndex);
				break;
			case UpArrow:
				ReadKeyValue(VK_UP, currentKeyIndex);
				break;
			case DownArrow:
				ReadKeyValue(VK_DOWN, currentKeyIndex);
				break;
			case LeftArrow:
				ReadKeyValue(VK_LEFT, currentKeyIndex);
				break;
			case RightArrow:
				ReadKeyValue(VK_RIGHT, currentKeyIndex);
				break;
			case F1:
				ReadKeyValue(VK_F1, currentKeyIndex);
				break;
			case F2:
				ReadKeyValue(VK_F2, currentKeyIndex);
				break;
			case F3:
				ReadKeyValue(VK_F3, currentKeyIndex);
				break;
			case F4:
				ReadKeyValue(VK_F4, currentKeyIndex);
				break;
			case F5:
				ReadKeyValue(VK_F5, currentKeyIndex);
				break;
			case F6:
				ReadKeyValue(VK_F6, currentKeyIndex);
				break;
			case F7:
				ReadKeyValue(VK_F7, currentKeyIndex);
				break;
			case F8:
				ReadKeyValue(VK_F8, currentKeyIndex);
				break;
			case F9:
				ReadKeyValue(VK_F9, currentKeyIndex);
				break;
			case F10:
				ReadKeyValue(VK_F10, currentKeyIndex);
				break;
			case F11:
				ReadKeyValue(VK_F11, currentKeyIndex);
				break;
			case F12:
				ReadKeyValue(VK_F12, currentKeyIndex);
				break;
			default:
				// ERROR! Out of range!
				break;
			}
		}
	}

	void InputManager::UpdateMouse()
	{
		if (mouseTracker & MouseTracker::WindowSelected)
		{
			if (GetForegroundWindow() != *windowHandle)
			{
				mouseDelta.x = 0;
				mouseDelta.y = 0;
				return;
			}
		}

		WINDOWPLACEMENT windowPlacement;
		POINT windowPosition;
		POINT& lastCursorPosition = mouseDelta;
		lastCursorPosition = mousePosition;

		GetWindowPlacement(*windowHandle, &windowPlacement);
		windowPosition.x = windowPlacement.rcNormalPosition.left;
		windowPosition.y = windowPlacement.rcNormalPosition.top;
		GetCursorPos(&mousePosition);


		if (mouseTracker & MouseTracker::OverWindows)
		{
			// Limit if not over window.
			if (!(mousePosition.x >= windowPlacement.rcNormalPosition.left && mousePosition.x <= windowPlacement.rcNormalPosition.right && mousePosition.y >= windowPlacement.rcNormalPosition.bottom && mousePosition.y <= (windowPlacement.rcNormalPosition.top - 32)))
			{
				mousePosition = lastCursorPosition;
				mouseDelta.x = 0;
				mouseDelta.y = 0;
				return;
			}
		}
		

		mousePosition.x -= windowPosition.x + 8;
		mousePosition.y -= (windowPosition.y + 32); // account for the title bar thingy

		mouseDelta.x = mousePosition.x - lastCursorPosition.x;
		mouseDelta.y = mousePosition.y - lastCursorPosition.y;


		// Read Mouse Buttons
	}

	void InputManager::ReadKeyValue(int virtualKey, short keyEnumOffset)
	{
		short currentKeyGetKeyState = GetKeyState(virtualKey);

		if (currentKeyGetKeyState & 0x8000)
		{
			if (keysPressed & (1 << keyEnumOffset))
			{
				// Needs to be held
				keysHeld = (InputFlags::KeyFlags)(keysHeld | (1 << keyEnumOffset));

				// No longer first frame pressing
				keysPressed = (InputFlags::KeyFlags)(keysPressed & ~(1 << keyEnumOffset));
			}
			else if (keysHeld & (1 << keyEnumOffset))
			{
				// It is being held. Nothing to do here
			}
			else
			{
				// Just pressed this frane
				keysPressed = (InputFlags::KeyFlags)(keysPressed | (1 << keyEnumOffset));
			}
		}
		else
		{
			if (keysHeld & (1 << keyEnumOffset))
			{
				// It was released
				keysHeld = (InputFlags::KeyFlags)(keysHeld & ~(1 << keyEnumOffset));

				keysReleased = (InputFlags::KeyFlags)(keysReleased | (1 << keyEnumOffset));
			}
			else if (keysPressed & (1 << keyEnumOffset))
			{
				// It was tapped for 1 frame. Therfore it was release
				keysPressed = (InputFlags::KeyFlags)(keysPressed & ~(1 << keyEnumOffset));

				keysReleased = (InputFlags::KeyFlags)(keysReleased | (1 << keyEnumOffset));
			}
			else if (keysReleased & (1 << keyEnumOffset))
			{
				// it weas released last frame
				keysReleased = (InputFlags::KeyFlags)(keysReleased & ~(1 << keyEnumOffset));
			}
		}
	}

	// TO;DO! MAKE USE MOUSE BUTTON STUFF!
	void InputManager::ReadMouseValue(int virtualKey, short keyEnumOffset)
	{
		short currentKeyGetKeyState = GetKeyState(virtualKey);

		if (currentKeyGetKeyState & 0x8000)
		{
			if (keysPressed & (1 << keyEnumOffset))
			{
				// Needs to be held
				keysHeld = (InputFlags::KeyFlags)(keysHeld | (1 << keyEnumOffset));

				// No longer first frame pressing
				keysPressed = (InputFlags::KeyFlags)(keysPressed & ~(1 << keyEnumOffset));
			}
			else if (keysHeld & (1 << keyEnumOffset))
			{
				// It is being held. Nothing to do here
			}
			else
			{
				// Just pressed this frane
				keysPressed = (InputFlags::KeyFlags)(keysPressed | (1 << keyEnumOffset));
			}
		}
		else
		{
			if (keysHeld & (1 << keyEnumOffset))
			{
				// It was released
				keysHeld = (InputFlags::KeyFlags)(keysHeld & ~(1 << keyEnumOffset));

				keysReleased = (InputFlags::KeyFlags)(keysReleased | (1 << keyEnumOffset));
			}
			else if (keysPressed & (1 << keyEnumOffset))
			{
				// It was tapped for 1 frame. Therfore it was release
				keysPressed = (InputFlags::KeyFlags)(keysPressed & ~(1 << keyEnumOffset));

				keysReleased = (InputFlags::KeyFlags)(keysReleased | (1 << keyEnumOffset));
			}
			else if (keysReleased & (1 << keyEnumOffset))
			{
				// it weas released last frame
				keysReleased = (InputFlags::KeyFlags)(keysReleased & ~(1 << keyEnumOffset));
			}
		}
	}
}