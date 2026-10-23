#pragma once

#include <iostream>
#include <Windows.h>
#include <string>

namespace Renderer
{
	class ConsoleRenderer
	{
	public:
		ConsoleRenderer(short rows, short columns, short xFontSize, short yFontSize, void(*Start)(), void(*Update)(float deltaTime), HANDLE* consolInputHandle, std::string consoleName = "CMD Renderer");
		~ConsoleRenderer();

		void Start();

		void DrawWCharacter(const short xPosition, const short yPosition, const wchar_t character);
		void DrawCharacter(const short xPosition, const short yPosition, const char character);

		void DrawWString(const short xPosition, const short yPosition, const std::wstring text);
		void DrawString(const short xPosition, const short yPosition, const std::string text);

		void DrawRectangleWCharacter(const short startXPosition, const short startYPosition, const short endXPosition, const short endYPosition, const wchar_t character);
		void DrawRectangleCharacter(const short startXPosition, const short startYPosition, const short endXPosition, const short endYPosition, const char character);


		void HideCurser(bool hidden);

		const HWND* GetWindowHandlePointer() const;

	private:
		HWND windowHandle;

		short width;
		short height;

		short fontWidth;
		short fontHeight;

		wchar_t* screenBuffer;

		DWORD numberOfCharactersWritten = 0;
		HANDLE consoleHandle;


		void(*OnStart)();
		void(*OnUpdate)(float deltaTime);

	};
}