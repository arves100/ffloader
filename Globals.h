/*!
	@date 06/02/2022
	@author Arves100
	@file Globals.h
	@brief project main singleton and global configuration
*/
#pragma once

#include "Loader.h"

class Globals
{
public:
	Globals();
	~Globals();

	static Globals* Get()
	{
		if (!ms_pSingleton)
			new Globals();

		return ms_pSingleton;
	}

	void Fatal(const wchar_t* error, const wchar_t* file, size_t line);

	// Public fields

	Loader* TheLoader;
	bool LoaderUseFullFunctions;
	HWND GameWindow;
	DWORD GamePID;
	HANDLE GameProcess;
	HINSTANCE GameModule;
	TCHAR GameDiskPath[MAX_PATH + 1];
	LPVOID BaseAddress;
	bool WindowedMode;

private:
	static Globals* ms_pSingleton;
};
