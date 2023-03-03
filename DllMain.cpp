/*!
    @file DllMain.cpp
    @date 01/11/2020
    @author Arves100
    @brief Loader entrypoint
*/
#include "StdAfx.h"
#include <Version.h>

#include "Globals.h"
#include "LDetours.h"

#ifdef _DEBUG
/*
    Initializes the debug console
    @return true in case of success, otherwise false
*/
static bool Console_Init()
{
    if (!AllocConsole())
    {
        return false;
    }

    // redirect output to consol
    FILE* d;
    freopen_s(&d, "CONOUT$", "w", stderr);
    freopen_s(&d, "CONOUT$", "w", stdout);

    SetConsoleTitle(L"Fur Fighters - Loader console");
    printf("Version %s (%s)\n", LOADER_VERSION, LOADER_BUILD);

    return TRUE;
}

/*
    Frees the debug console
*/
static void Console_Free()
{
    FreeConsole();
}
#endif

/*
    Loader DLL entrypoint
    @param hinstDLL Current DLL instance
    @param fdwReason Reason for calling this callback
    @param lpReserved Reserved parameter
    @return TRUE in case the call was succeeded, otherwise FALSE
*/
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
