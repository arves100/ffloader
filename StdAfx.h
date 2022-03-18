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

#ifndef DLLAPI
#define DLLAPI __declspec(dllexport)
#endif

#ifndef HRESULT_INT
#define HRESULT_INT int // typedef edit for integration with decomp
#endif

// DirectX 8 SDK inclusions
#define DIRECTINPUT_VERSION 0x700
#include <ddraw.h>
#include <dplay.h>
#include <dinput.h>
#include <dplobby.h>

#define CRASH { int* x = 0; *x = 999; }

#ifdef _DEBUG
#define FATAL(x) Globals::Get()->Fatal(L##x, __FILEW__, __LINE__)
#define FATAL2(x) Globals::Get()->Fatal(x, __FILEW__, __LINE__)
#else
#define FATAL(x) Globals::Get()->Fatal(L##x, L"", 0)
#define FATAL2(x) Globals::Get()->Fatal(x, L"", 0)
#endif

#define REGISTRY_KEY L"SOFTWARE\\Bizarre Creations\\Fur Fighters\\Loader"

// {CC84BC6B-DF98-4097-9429-21AC86426A42}
DEFINE_GUID(GUID_ENet, 0xcc84bc6b, 0xdf98, 0x4097, 0x94, 0x29, 0x21, 0xac, 0x86, 0x42, 0x6a, 0x42);

// {68DCDBFF-BA9C-4776-BC9B-BB98CB9A276A}
DEFINE_GUID(GUID_ENetAddress, 0x68dcdbff, 0xba9c, 0x4776, 0xbc, 0x9b, 0xbb, 0x98, 0xcb, 0x9a, 0x27, 0x6a);

// C
#include <cstring>
#include <cstdlib>

// C++
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <thread>

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

// ENet Network
#include "enet.h"

// GUID hash function (required for FakeDP)
struct GUIDHasher
{
	size_t operator()(const GUID& k) const
	{
		RPC_STATUS r;
		return UuidHash((UUID*) &k, &r);
	}
};
