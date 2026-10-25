#pragma once

#include "../Engine/Renderer/ConsoleRenderer/ConsoleRenderer.h"
#include "../Engine/Input/Win32Input.h"

namespace Games::Celeste
{
	/*
	enum Tile : __int8
	{
		Air = 1 << 0,
		Solid = 1 << 1,
		Spike_Roof = 1 << 2,
		Spike_Ground = 1 << 3,
		Spike_Roof = 1 << 4,
		Spike_RightWall = 1 << 5,
		Spike_LeftWall = 1 << 6,
		Transition = 1 << 7,
	};

	struct Room
	{
		virtual const char* GetRoomString() const;
	};

	struct Room1 : Room
	{
		const char* GetRoomString() const override 
		{
			return	
				"##############################     #####"
				"##### ###    #########             #####"
				"##  #   #    #####   #             #####"
				"##  ### #    ###     #             #####"
				"##  #   #    ###                  ######"
				"##  #   #    ###                   #####"
				"##  #####                          #####"
				"##                            ##########"
				"##                            ##########"
				"#                             ##########"
				"#                 ####           <######"
				"#                 #  #           <######"
				"#                 #  #              ####"
				"#                 ####              ####"
				"#                 # ##              ####"
				"#                 ####^^^^          ####"
				"#         ###     ########           ###"
				"#         ###     ########           ###"
				"#####     ###^^^^^########            ##"
				"#####     ################^^^          #"
				"#####^^^^^###################          #"
				"#############################          #";
		}
	};

	struct Map
	{
		Room1 room1;
	};
	*/
	class Celeste
	{
	public:
		Celeste();
		~Celeste();

		void Start();
		void Update(float deltaTime);

	private:
		void Render();
		HANDLE consoleInputHandle = nullptr;

		Renderer::ConsoleRenderer renderer;
		Input::InputManager* inputManager;
	};
}