#define GLEW_STATIC
#include <iostream>
#include <windows.h>

#include <fstream>
#include <gamemachine.h>
#include <gmgl.h>
#include <gmui.h>
#include <gmm.h>
#include "demostration_world.h"

#define EMITTER_DEMO 1

using namespace gm;

GMGLFactory factory;

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	char * lpCmdLine,
	int nCmdShow
)
{
	GMWindowAttributes mainAttrs =
	{
		NULL,
		L"Default",
		WS_OVERLAPPEDWINDOW,
		0,
		{ 0, 0, 800, 600 },
		NULL,
	};

	gm::IWindow* mainWindow = nullptr;
	gmui::GMUIFactory::createMainWindow(hInstance, &mainWindow);
	mainWindow->create(mainAttrs);

	GMWindowAttributes consoleAttrs =
	{
		NULL,
		L"GameMachineConsoleWindow",
		WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU,
		WS_EX_CLIENTEDGE,
		{ 0, 0, 700, 400 },
		NULL,
	};

	gm::GMConsoleHandle consoleHandle;
	gmui::GMUIFactory::createConsoleWindow(hInstance, consoleHandle);
	consoleHandle.window->create(consoleAttrs);

	DemostrationEntrance* entrance = new DemostrationEntrance();
	DemostrationWorld& world = entrance->getWorld();

	gm::IAudioPlayer* player = nullptr;
	gmm::GMMFactory::createAudioPlayer(&player);

	GM.init(
		mainWindow,
		consoleHandle,
		player,
		new GMGLFactory(),
		entrance
	);

	GameMachine::instance().startGameMachine();
	return 0;
}