/*!
	@date 01/11/2020
	@author Arves100
	@file LDetours.cpp
	@brief Detours functions
*/
#include "StdAfx.h"
#include "Detours.h"
#include "Globals.h"

#include <detours.h>

struct DetoursFnc
{
	PVOID& original;
	PVOID detour;
};

#define DETOURS_ADD(name) { (PVOID&)Original_##name, Detour_##name }
#define DETOURS_FNC(name, param, ret) static ret(WINAPI*Original_##name) param = name; static ret WINAPI Detour_##name param

DETOURS_FNC(CreateWindowExA, (DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam), HWND)
{
	bool isWinny = strcmp(lpClassName, "WINNY") == 0;

	if (isWinny)
	{
		// We need to patch here or g_dwWindowedMode will be resetted again
		Globals::Get()->TheLoader->ApplyInitPatch();

		if (!Globals::Get()->WindowedMode)
		{
			dwStyle &= ~WS_OVERLAPPEDWINDOW; // PATCH: do not apply a windowed style when you are playing in fullscreen
		}
		else
		{
			dwStyle &= ~WS_POPUP; // PATCH: Do not flag the window as a popup when being windowed

			// PATCH: add the missing border size so the screen size is corrected
			nWidth += GetSystemMetrics(SM_CXSIZEFRAME);
			nHeight += GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYSMCAPTION);
		}

		dwExStyle &= ~WS_EX_TOPMOST; // PATCH: Do not keep the application on topmost
	}

	HWND hWnd = Original_CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	if (!hWnd)
		return nullptr;

	if (isWinny)
	{
		// Intercept game window
		Globals::Get()->GameWindow = hWnd;
	}

	return hWnd;
}

static BOOL WINAPI ScreenModeDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return Globals::Get()->TheLoader->ScreenModeDialogProc(hDlg, Msg, wParam, lParam);
}

DETOURS_FNC(DialogBoxParamA, (_In_opt_ HINSTANCE hInstance, _In_ LPCSTR lpTemplateName, _In_opt_ HWND hWndParent, _In_opt_ DLGPROC lpDialogFunc, _In_ LPARAM dwInitParam), INT_PTR)
{
	// Modify the dialog system
	if (lpTemplateName == (LPCSTR)144)
	{
		Globals::Get()->TheLoader->SetScreenModeDlgProc(lpDialogFunc);
		return Original_DialogBoxParamA(hInstance, lpTemplateName, NULL, ScreenModeDialogProc, dwInitParam); // PATCHED: Change DlgProc and avoid using "GetForegroundWindow" as a parent window
	}

	return Original_DialogBoxParamA(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
}

static LRESULT WINAPI GameWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return Globals::Get()->TheLoader->GameWindowProc(hWnd, Msg, wParam, lParam);
}

DETOURS_FNC(RegisterClassA, (_In_ const WNDCLASSA* lpWndClass), ATOM)
{
	if (strcmp(lpWndClass->lpszClassName, "WINNY") == 0)
	{
		WNDCLASSA* newClass = const_cast<WNDCLASSA*>(lpWndClass);

		Globals::Get()->TheLoader->SetGameWindowProc(lpWndClass->lpfnWndProc);
		newClass->lpfnWndProc = GameWindowProc;
	}

	return Original_RegisterClassA(lpWndClass);
}

DETOURS_FNC(RegQueryValueExA, (_In_ HKEY hKey,_In_opt_ LPCSTR lpValueName, LPDWORD lpReserved, _Out_opt_ LPDWORD lpType, _Out_opt_ LPBYTE lpData, _Inout_opt_ LPDWORD lpcbData), LSTATUS)
{
	if (strcmp(lpValueName, "Controller Configuration") == 0)
	{
		Globals::Get()->TheLoader->InitDirectInputData();
	}

	return Original_RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

static DetoursFnc DETOURS[] =
{
	DETOURS_ADD(DialogBoxParamA),
	DETOURS_ADD(CreateWindowExA),
	DETOURS_ADD(RegisterClassA),
	DETOURS_ADD(RegQueryValueExA),
};

void DetourInit(void)
{
	//DetourRestoreAfterWith();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	for (size_t i = 0; i < ARRAYSIZE(DETOURS); i++)
		DetourAttach(&DETOURS[i].original, DETOURS[i].detour);

	DetourTransactionCommit();
}

void DetourDeinit(void)
{
	//DetourRestoreAfterWith();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	for (size_t i = 0; i < ARRAYSIZE(DETOURS); i++)
		DetourDetach(&DETOURS[i].original, DETOURS[i].detour);

	DetourTransactionCommit();
}
