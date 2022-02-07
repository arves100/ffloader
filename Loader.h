/*!
	@date 06/02/2022
	@author Arves100
	@file Loader.h
	@brief Main loader instance
*/
#pragma once

class Loader
{
public:
	Loader() = default;
	~Loader() = default;

	bool Init();

	BOOL ScreenModeDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
	void SetScreenModeDlgProc(DLGPROC dlgProc) { m_screenDlgProc = dlgProc; }
	void AddScreenMode(LPDDSURFACEDESC2 dd);
	void SetGameWindowProc(WNDPROC wndProc) { m_gameWndProc = wndProc; }
	LRESULT GameWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	void ApplyInitPatch();

private:
	void CheckSha256OfGame();
	void ApplyPreInitPatch();
	void PatchScreenMode();
	void CreateOrLoadSettings();
	void SaveSettings();

	DLGPROC m_screenDlgProc;
	DWORD m_dwSelectedScreenMode;
	std::vector<avail_display_info> m_vModes;
	bool m_bSkipIntro;
	bool m_bNoCd;
	DWORD m_dwGpu;
	WNDPROC m_gameWndProc;

	// DIALOG HWND
	HWND hExtra;
	HWND hSkipIntro;
	HWND hWindowed;
};
