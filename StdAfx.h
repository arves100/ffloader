/*!
	@file StdAfx.h
	@brief Main inclusion of the project
	@author Arves100
	@date 01/11/2020
*/
#pragma once

#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <Psapi.h>

#define DLLAPI __declspec(dllexport)

typedef int HRESULT_INT;

// DirectX 8 SDK inclusions
#include <ddraw.h>
#include <dplay.h>

#define CRASH { int* x = 0; *x = 999; }

#ifdef _DEBUG
#define FATAL(x) Globals::Get()->Fatal(L##x, __FILEW__, __LINE__)
#define FATAL2(x) Globals::Get()->Fatal(x, __FILEW__, __LINE__)
#else
#define FATAL(x) Globals::Get()->Fatal(L##x, L"", 0)
#define FATAL2(x) Globals::Get()->Fatal(x, L"", 0)
#endif

#define REGISTRY_KEY L"SOFTWARE\\Bizarre Creations\\Fur Fighters\\Loader"

// C
#include <cstring>
#include <cstdlib>

// C++
#include <string>
#include <vector>

// REVERSED: CONTENT OF ARRAY AT 0x005B1DA8
struct avail_display_info
{
	DWORD dwWidth;
	DWORD dwheight;
	DWORD dwRGBBitsCount;

	constexpr bool operator==(const avail_display_info& lhs) const
	{
		return lhs.dwheight == dwheight && lhs.dwWidth == dwWidth && lhs.dwRGBBitsCount == dwRGBBitsCount;
	}
};
