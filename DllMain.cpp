/*!
    @author Arves100
    @brief DLL Entrypoint
    @date 01/11/2020
*/
#include "StdAfx.h"
#include "Globals.h"
#include "LDetours.h"

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        if (!Globals::Get()->TheLoader->Init())
            return FALSE;

        DetourInit();
        break;

    default:
        break;

    case DLL_PROCESS_DETACH:
        DetourDeinit();
        delete Globals::Get();
        break;
    }

    return TRUE;
}
