#include "Celeste.h"
#include "Engine/Input/Win32Input.h"

Games::Celeste::Celeste::Celeste()
	: renderer(40, 22, 20, 20, [this]() { this->Start(); }, [this](float deltaTime) { this->Update(deltaTime); }, "Celeste CMD")
{
	inputManager = new Input::InputManager(renderer.GetWindowHandlePointer());

	renderer.Start();
}

Games::Celeste::Celeste::~Celeste()
{}

void Games::Celeste::Celeste::Start()
{
	
}

void Games::Celeste::Celeste::Update(float deltaTime)
{
	Render();
}

void Games::Celeste::Celeste::Render()
{
	std::string mapString = 
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

	for (int i = 0; i < 40 * 22; i++)
	{
		if (mapString[i] == ' ')
		{
			renderer.DrawCharacter(i % 40, i / 40, ' ');
		}
		if (mapString[i] == '#')
		{
			renderer.DrawCharacter(i % 40, i / 40, '#');
		}
		if (mapString[i] == '^')
		{
			renderer.DrawCharacter(i % 40, i / 40, '^');
		}
		if (mapString[i] == '<')
		{
			renderer.DrawCharacter(i % 40, i / 40, '<');
		}
	}
}
