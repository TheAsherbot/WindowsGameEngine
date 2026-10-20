#include "Win32Input.h"

#define KEY_PRESSED 0x8000 

namespace Input
{
	InputManager::InputManager()
	{
		keysRegistered = (KeyFlags)0;
		keysHeld = (KeyFlags)0;
		keysPressed = (KeyFlags)0;
		keysReleased = (KeyFlags)0;
	}
	InputManager::~InputManager()
	{
	}
	
	
	void InputManager::Update()
	{
		int keyID = 0;

		switch (key)
		{
		case A:
			break;
		case B:
			break;
		case C:
			break;
		case D:
			break;
		case E:
			break;
		case F:
			break;
		case G:
			break;
		case H:
			break;
		case I:
			break;
		case J:
			break;
		case K:
			break;
		case L:
			break;
		case M:
			break;
		case N:
			break;
		case O:
			break;
		case P:
			break;
		case Q:
			break;
		case R:
			break;
		case S:
			break;
		case T:
			break;
		case U:
			break;
		case V:
			break;
		case W:
			break;
		case X:
			break;
		case Y:
			break;
		case Z:
			break;
		case One:
			break;
		case Two:
			break;
		case Three:
			break;
		case Four:
			break;
		case Five:
			break;
		case Six:
			break;
			break;
		case Seven:
			break;
		case Eight:
			break;
		case Nine:
			break;
		case Zero:
			break;
		case Ctrl:
			break;
		case LCtrl:
			break;
		case RCtrl:
			break;
		case Shift:
			break;
		case LShift:
			break;
		case RShift:
			break;
		case Alt:
			break;
		case LAlt:
			break;
		case RAlt:
			break;
		case Esc:
			break;
		case Tab:
			break;
		case Enter:
			break;
		case UpArrow:
			break;
		case DownArrow:
			break;
		case LeftArrow:
			break;
		case RightArrow:
			break;
		case F1:
			break;
		case F2:
			break;
		case F3:
			break;
		case F4:
			break;
		case F5:
			break;
		case F6:
			break;
		case F7:
			break;
		case F8:
			break;
		case F9:
			break;
		case F10:
			break;
		case F11:
			break;
		case F12:
			break;
		default:
			// ERROR! Out of range!
			break;
		}
	}
	
	void InputManager::ReadKeyValue(int virtualKey, char keyEnumOffset)
	{
		short currentKeuGetKeyState = GetKeyState(virtualKey);

		if (currentKeuGetKeyState & KEY_PRESSED)
		{
			if (keysPressed & (1 << keyEnumOffset))
			{
				// needs to be held
			}
			// if already held then do nothing, if not then pressed
		}

		// check if reelased
	}
}