#pragma once

#include <Windows.h>
#include "Engine/Renderer/ConsoleRenderer/ConsoleRenderer.h"

namespace Input
{
	namespace InputFlags
	{
		enum KeyFlags : long long // 64 options 2 ints are 64 bits. long long = 64 bits
		{
			A = 1 << 0,
			B = 1 << 1,
			C = 1 << 2,
			D = 1 << 3,
			E = 1 << 4,
			F = 1 << 5,
			G = 1 << 6,
			H = 1 << 7,
			I = 1 << 8,
			J = 1 << 9,
			K = 1 << 10,
			L = 1 << 11,
			M = 1 << 12,
			N = 1 << 13,
			O = 1 << 14,
			P = 1 << 15,
			Q = 1 << 16,
			R = 1 << 17,
			S = 1 << 18,
			T = 1 << 19,
			U = 1 << 20,
			V = 1 << 21,
			W = 1 << 22,
			X = 1 << 23,
			Y = 1 << 24,
			Z = 1 << 25,
			One = 1 << 26,
			Two = 1 << 27,
			Three = 1 << 28,
			Four = 1 << 29,
			Five = 1 << 30,
			Six = 1 << 31,
			Seven = (long long)1 << 32,
			Eight = (long long)1 << 33,
			Nine = (long long)1 << 34,
			Zero = (long long)1 << 35,
			Ctrl = (long long)1 << 36,
			LCtrl = (long long)1 << 37,
			RCtrl = (long long)1 << 38,
			Shift = (long long)1 << 39,
			LShift = (long long)1 << 40,
			RShift = (long long)1 << 41,
			Alt = (long long)1 << 42,
			LAlt = (long long)1 << 43,
			RAlt = (long long)1 << 44,
			Esc = (long long)1 << 45,
			Tab = (long long)1 << 46,
			Enter = (long long)1 << 47,
			UpArrow = (long long)1 << 48,
			DownArrow = (long long)1 << 49,
			LeftArrow = (long long)1 << 50,
			RightArrow = (long long)1 << 51,
			F1 = (long long)1 << 52,
			F2 = (long long)1 << 53,
			F3 = (long long)1 << 54,
			F4 = (long long)1 << 55,
			F5 = (long long)1 << 56,
			F6 = (long long)1 << 57,
			F7 = (long long)1 << 58,
			F8 = (long long)1 << 59,
			F9 = (long long)1 << 60,
			F10 = (long long)1 << 61,
			F11 = (long long)1 << 62,
			F12 = (long long)1 << 63,
		};

		/// <summary>
		/// Every key uses 2 bits.
		/// 00 - Not pressed
		/// 01 - Held
		/// 11 - Pressed
		/// 10 - Released
		/// </summary>
		enum MouseButtonFlags : unsigned short
		{
			// These actually use the same keybaord detection stuff.
			Left = 1,
			Middle = 4,
			Right = 16,
			Side1 = 64,
			Side2 = 256,
		};
	}
	enum Key // 64 options
	{
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		One,
		Two,
		Three,
		Four,
		Five,
		Six,
		Seven,
		Eight,
		Nine,
		Zero,
		Ctrl,
		LCtrl,
		RCtrl,
		Shift,
		LShift,
		RShift,
		Alt,
		LAlt,
		RAlt,
		Esc,
		Tab,
		Enter,
		UpArrow,
		DownArrow,
		LeftArrow,
		RightArrow,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
	};

	enum KeyState : unsigned short
	{
		NotPressed,
		Pressed,
		Held,
		Released,
		NotRegistered
	};

	enum MouseButton : unsigned short
	{
		Left,
		Middle,
		Right,
		Side1,
		Side2,
	};

	// Set Both to true to have it only track mouse when the window is the selected window, and it is over the current window.
	enum MouseTracker : short
	{
		WindowSelected = 1,
		OverWindows = 2,
	};


	class InputManager
	{
	public:

		InputManager(const HWND* windowHandle);
		~InputManager();

		void Update();

		void RegisterKeys(InputFlags::KeyFlags keys);
		void RegisterKey(Key key);
		void UnregisterKeys(InputFlags::KeyFlags keys);
		void UnregisterKey(Key key);
		InputFlags::KeyFlags GetRegisteredKeys() const;
		bool IsKeyRegistered(Key key) const;

		KeyState GetRegisteredKeyState(Key key) const;

		KeyState GetMouseButtonState(MouseButton mouseButton) const;
		void SetMouseTracker(MouseTracker mouseTracker);
		const POINT* GetMousePositionPointer() const;
		const POINT* GetMousePositionDeltaPointer() const;



	private:
		const HWND* windowHandle;

		InputFlags::KeyFlags keysRegistered; // only registered keys will be checked for input to save time.

		InputFlags::KeyFlags keysHeld; // Stores all the registered keys that are held. 1 is true 0 false. Iis not held first fram that is pressed
		InputFlags::KeyFlags keysPressed; // Stores all the registered keys that are pressed this frame. 1 is true, 0 false.
		InputFlags::KeyFlags keysReleased; // Stores all the registered keys that were held last frame is are not anymore.

		MouseTracker mouseTracker;
		InputFlags::MouseButtonFlags mouseButtons;
		POINT mousePosition;
		POINT mouseDelta;

		void UpdateRegisteredKeys();
		void UpdateMouse();


		void ReadKeyValue(int virutalKey, short keyEnumOffset);
		void ReadMouseValue(int virutalKey, short keyEnumOffset);
	};
}