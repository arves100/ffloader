/*!
	@date 06/02/2022
	@author Arves100
	@file Loader.cpp
	@brief Main loader instance
*/
#include "StdAfx.h"
#include "Loader.h"
#include "Globals.h"

// Game patches
#include "patches/nocd.h"
#include "patches/nointro.h"
#include "patches/enumalldisplay.h"

bool Loader::Init()
{
	auto globals = Globals::Get();

	// Process ID, first step
	globals->GamePID = GetCurrentProcessId();
	globals->GameProcess = GetCurrentProcess();
	globals->GameModule = GetModuleHandleW(L"furfighters.exe");

	if (globals->GamePID == 0 || !globals->GameProcess || !globals->GameModule)
	{
		FATAL("Cannot get game process information");
	}

	if (GetModuleFileName(globals->GameModule, globals->GameDiskPath, sizeof(globals->GameDiskPath) / sizeof(TCHAR)) == 0)
	{
		FATAL("Cannot get game executable position");
	}

	MODULEINFO info;
	if (!GetModuleInformation(globals->GameProcess, globals->GameModule, &info, sizeof(info)))
	{
		FATAL("Cannot get game module information");
	}

	globals->BaseAddress = info.lpBaseOfDll;

	CreateOrLoadSettings();

	globals->LoaderUseFullFunctions = true;

	if (globals->LoaderUseFullFunctions)
		ApplyPreInitPatch();

	return true;
}

void Loader::ApplyPreInitPatch()
{
	if (m_bNoCd)
		ApplyNOCD();

	ApplyENUMALLDISPLAY();
}

void Loader::CheckSha256OfGame()
{
#if 0
	auto globals = Globals::Get();
	const wchar_t* shaFailMsg = nullptr;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	std::wstring warnMsg = L"The loader was unable to verify the correct integrity of the Fur Fighters executable.\nThe game will still start and network functionalities will still work,\nbut you will not be able to use all the loader features that involves patching the game.\n\nError: ";

	char* buffer = new char[SHA256_CHECK_BUFFER_SIZE];
	DWORD readed;
	DWORD total;
	SHA256_CTX ctx;
	BYTE hashOut[SHA256_BLOCK_SIZE];
	std::string strHash;

	hFile = CreateFileW(globals->GameDiskPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);

	if (!hFile)
	{
		shaFailMsg = L"Unable to open the file on the disk drive";
		goto exit;
	}

	sha256_init(&ctx);

	total = GetFileSize(hFile, NULL);

	warnMsg = std::to_wstring(total);
	MessageBoxW(nullptr, warnMsg.c_str(), L"", 0);


	for (DWORD i = 0; i < total; i += readed)
	{
		DWORD tr = total - i;
		if (tr > SHA256_CHECK_BUFFER_SIZE)
			tr = SHA256_CHECK_BUFFER_SIZE;

		if (!ReadFile(hFile, buffer, tr, &readed, nullptr))
		{
			shaFailMsg = L"Unable to read Fur Fighters file";
			goto exit;
		}

		sha256_update(&ctx, reinterpret_cast<const BYTE*>(buffer), readed);
	}

	sha256_final(&ctx, hashOut);

	strHash = make_hex_string(hashOut, hashOut + SHA256_BLOCK_SIZE);

	if (strcmp(strHash.c_str(), FF12_SHA256) != 0)
	{
		shaFailMsg = L"Invalid hash";

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		warnMsg = converter.from_bytes(strHash);

		goto exit;
	}

	globals->LoaderUseFullFunctions = true;
	goto exit;

exit:
	delete[] buffer;

	if (hFile)
		CloseHandle(hFile);

	if (shaFailMsg)
	{
		warnMsg += shaFailMsg;
		MessageBoxW(globals->GameWindow, warnMsg.c_str(), L"Warning", MB_OK | MB_ICONWARNING);
	}

	CRASH;
#endif
}

void Loader::AddScreenMode(LPDDSURFACEDESC2 dd)
{
	avail_display_info di;
	di.dwheight = dd->dwHeight;
	di.dwWidth = dd->dwWidth;
	di.dwRGBBitsCount = dd->ddpfPixelFormat.dwRGBBitCount;

	for (auto x = m_vModes.begin(); x != m_vModes.end(); x++)
	{
		if (*x == di)
			return;
	}

	m_vModes.push_back(di);
}

BOOL Loader::ScreenModeDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == 1) // BUTTON_OK
		{
			HWND hComboScreenModes = GetDlgItem(hDlg, 0x40E);
			m_dwSelectedScreenMode = SendMessage(hComboScreenModes, CB_GETCURSEL, 0, 0);

			if ((m_dwSelectedScreenMode + 1) > m_vModes.size())
			{
				MessageBox(hDlg, L"The specified display mode does not exist", L"Error", MB_OK | MB_ICONERROR);
				return TRUE;
			}

			auto v = SendMessage(hSkipIntro, BM_GETCHECK, 0, 0);

			if (v == BST_CHECKED)
				m_bSkipIntro = true;
			else
				m_bSkipIntro = false;

			HWND hGpu = GetDlgItem(hDlg, 0x3FA);// Combo box device selection
			m_dwGpu = SendMessage(hGpu, CB_GETCURSEL, 0, 0);

			v = SendMessage(hWindowed, BM_GETCHECK, 0, 0);

			if (v == BST_CHECKED)
				Globals::Get()->WindowedMode = true;
			else
				Globals::Get()->WindowedMode = false;

			PatchScreenMode();
		}
		break;
	case WM_INITDIALOG:
	{
		Globals::Get()->GameWindow = hDlg;

		if ((m_dwSelectedScreenMode + 1) > m_vModes.size())
			m_dwSelectedScreenMode = 0; // reset in case of bad value

		RECT rect;
		GetWindowRect(hDlg, &rect);
		rect.bottom += 55;
		SetWindowPos(hDlg, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);

		HFONT hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);

		hExtra = CreateWindow(L"BUTTON", L"Extr&a settings", BS_GROUPBOX | WS_CHILD | WS_VISIBLE, 7, 130, 190, 60, hDlg, nullptr, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), 0);
		hWindowed = CreateWindow(L"BUTTON", L"&Windowed mode", BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 13, 147, 100, 20, hDlg, nullptr, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), 0);
		hSkipIntro = CreateWindow(L"BUTTON", L"&Skip intro", BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 13, 167, 100, 20, hDlg, nullptr, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), 0);
		
		if (!hExtra || !hWindowed || !hSkipIntro)
			return FALSE;

		// Fix font
		SendMessage(hExtra, WM_SETFONT, (WPARAM)hFont, (LPARAM)1);
		SendMessage(hWindowed, WM_SETFONT, (WPARAM)hFont, (LPARAM)1);
		SendMessage(hSkipIntro, WM_SETFONT, (WPARAM)hFont, (LPARAM)1);

		if (m_bSkipIntro)
			SendMessage(hSkipIntro, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

		if (Globals::Get()->WindowedMode)
			SendMessage(hWindowed, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

		break;
	}
	case WM_DESTROY:
		DestroyWindow(hExtra);
		DestroyWindow(hWindowed);
		DestroyWindow(hSkipIntro);
		Globals::Get()->GameWindow = nullptr; // reset game window as the dialog is destroyed now

		SaveSettings();

		break;
	}

	auto r =  m_screenDlgProc(hDlg, Msg, wParam, lParam);

	if (Msg == WM_INITDIALOG) // Apply settings specific changes here
	{
		HWND hScreenModes = GetDlgItem(hDlg, 0x40E); // Combobox screen modes
		HWND hGpu = GetDlgItem(hDlg, 0x3FA);// Combo box device selection

		SendMessage(hGpu, CB_SETCURSEL, m_dwGpu, 0);
		SendMessage(hScreenModes, CB_SETCURSEL, m_dwSelectedScreenMode, 0);
	}

	return r;
}

void Loader::PatchScreenMode()
{
	auto globals = Globals::Get();
	auto addr = (LPBYTE)globals->BaseAddress + (SUPPORTED_SCREEN_MODE_POSITION - IDA_BASE);
	SIZE_T written;

	if (!WriteProcessMemory(globals->GameProcess, addr, &m_vModes[m_dwSelectedScreenMode], sizeof(avail_display_info), &written))
		FATAL("Unable to apply display mode settings");

	if (written != sizeof(avail_display_info))
		FATAL("Unable to properly apply display mode settings");
}

void Loader::ApplyInitPatch()
{
	auto globals = Globals::Get();

	if (m_bSkipIntro)
		ApplyNOINTRO();

	if (globals->WindowedMode)
	{
		auto addr = (LPBYTE)globals->BaseAddress + (WINDOWED_MODE_POSITION - IDA_BASE);
		SIZE_T wri = 0;
		int tmp = 1;

		if (!WriteProcessMemory(globals->GameProcess, addr, &tmp, 4, &wri) || wri != 4)
			FATAL("Unable to set window mode in game");
	}
}

void Loader::CreateOrLoadSettings()
{
	HKEY regKey;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_KEY, 0, nullptr, REG_OPTION_NON_VOLATILE, (REGSAM)0xF003F, nullptr, &regKey, nullptr) != ERROR_SUCCESS)
	{
		FATAL("Cannot load the loader registry key");
	}

	DWORD data = 0, sz = 4;
	if (RegQueryValueEx(regKey, L"Real Display settings", nullptr, nullptr, (LPBYTE)&data, &sz) == ERROR_SUCCESS)
	{
		m_dwSelectedScreenMode = data;
	}

	data = 0;
	if (RegQueryValueEx(regKey, L"NoIntro", nullptr, nullptr, (LPBYTE)&data, &sz) == ERROR_SUCCESS)
	{
		m_bSkipIntro = data > 0;
	}

	if (RegQueryValueEx(regKey, L"GPU", nullptr, nullptr, (LPBYTE)&data, &sz) == ERROR_SUCCESS)
	{
		m_dwGpu = data;
	}

	if (RegQueryValueEx(regKey, L"Windowed", nullptr, nullptr, (LPBYTE)&data, &sz) == ERROR_SUCCESS)
	{
		Globals::Get()->WindowedMode = data > 0;
	}

	if (RegQueryValueEx(regKey, L"NoCD", nullptr, nullptr, (LPBYTE)&data, &sz) == ERROR_SUCCESS)
	{
		m_bNoCd = data;
	}
	else
		m_bNoCd = true; // default is true

	RegCloseKey(regKey);
}

void Loader::SaveSettings()
{
	HKEY regKey;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_KEY, 0, nullptr, REG_OPTION_NON_VOLATILE, (REGSAM)0xF003F, nullptr, &regKey, nullptr) != ERROR_SUCCESS)
	{
		FATAL("Cannot save the loader registry key");
	}

	RegSetValueEx(regKey, L"Real Display settings", 0, REG_DWORD, (LPBYTE)&m_dwSelectedScreenMode, 4);
	DWORD tmp = m_bSkipIntro;
	RegSetValueEx(regKey, L"NoIntro", 0, REG_DWORD, (LPBYTE)&tmp, 4);
	RegSetValueEx(regKey, L"GPU", 0, REG_DWORD, (LPBYTE)&m_dwGpu, 4);
	tmp = Globals::Get()->WindowedMode;
	RegSetValueEx(regKey, L"Windowed", 0, REG_DWORD, (LPBYTE)&tmp, 4);

	RegCloseKey(regKey);
}

LRESULT Loader::GameWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return m_gameWndProc(hWnd, Msg, wParam, lParam);
}
