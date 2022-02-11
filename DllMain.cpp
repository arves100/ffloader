/*!
    @author Arves100
    @brief DLL Entrypoint
    @date 01/11/2020
*/
#include "StdAfx.h"
#include "Globals.h"
#include "LDetours.h"

#ifdef _DEBUG
static BOOL InitDbgConsole()
{
    if (!AllocConsole())
    {
        MessageBox(nullptr, L"Unable to initialize the debug console", L"Fatal error", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    FILE* d;
    freopen_s(&d, "CONOUT$", "w", stderr);
    freopen_s(&d, "CONOUT$", "w", stdout);

    SetConsoleTitle(L"Fur Fighters - Loader console");
    printf("Build: %s (Arves100)\n", __TIMESTAMP__);

    return TRUE;
}

static void DelDbgConsole()
{
    FreeConsole();
}
#endif

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
        if (!InitDbgConsole())
            return FALSE;
#endif
        if (!Globals::Get()->TheLoader->Init())
            return FALSE;

        DetourInit();
        break;

    default:
        break;

    case DLL_PROCESS_DETACH:
        DetourDeinit();
        delete Globals::Get();
#ifdef _DEBUG
        DelDbgConsole();
#endif
        break;
    }

    return TRUE;
}
