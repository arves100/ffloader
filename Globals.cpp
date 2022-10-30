/*!
	@date 06/02/2022
	@author Arves100
	@file Globals.cpp
	@brief project main singleton and global configuration
*/
#include "StdAfx.h"
#include "Globals.h"

#define DP_ARENA_SIZE 1024*1024*50 // 50MB

Globals::~Globals()
{
	delete TheLoader;
	ms_pSingleton = nullptr;
}

Globals::Globals()
{
	ms_pSingleton = this;

	TheLoader = new Loader();
	LoaderUseFullFunctions = false;
	GameWindow = nullptr;
	GamePID = 0;
	GameProcess = nullptr;
	GameModule = nullptr;
	memset(GameDiskPath, 0, sizeof(GameDiskPath));
	BaseAddress = nullptr;
	WindowedMode = false;
	TheArena = new DPMsgArena(DP_ARENA_SIZE); // 20MB

	if (!TheLoader || !TheArena)
		FATAL("Unable to allocate loader memory");
}

Globals* Globals::ms_pSingleton = nullptr;

void Globals::Fatal(const wchar_t* error, const wchar_t* file, size_t line)
{
	std::wstring rsz = L"Unable to initialize the loader!\nPlease check if the installation was correct, and the game was launched with administrator privileges\notherwise, please contact the developers or open an issue.\n\n";

	rsz += L"Error: ";
	rsz += error;
	rsz += L"\n";

#ifdef _DEBUG
	rsz += L"File: ";
	rsz += file;
	rsz += L"\nLine: ";
	rsz += std::to_wstring(line);
	rsz += L"\n";
#endif
	rsz += L"\n";
	rsz += L"By pressing OK, the game will now close. (There might be a crash error, don't worry it's normal just ignore it and don't send any report)";

	MessageBoxW(GameWindow, rsz.c_str(), L"Loader error", MB_OK | MB_ICONERROR);
	CRASH;
}
