/*!
	@date 07/02/2022
	@author Arves100
	@file ExportConn.cpp
	@brief Exports functions that the patched game will load
*/
#include "StdAfx.h"
#include "Globals.h"

// Used in DirectDraw Screen surface enumations 
extern "C" void DLLAPI D3DXERR_ENUMFORMATSFAILED(LPDDSURFACEDESC2 lpDesc, HMODULE lib)
{
	Globals::Get()->TheLoader->AddScreenMode(lpDesc);
	FreeLibrary(lib);
}

#if 0
// Fur Fighters's time() function seems to cause errors
extern "C" time_t DLLAPI D3DXERR_SURFACENOTPALETTED(time_t * t, HMODULE lib)
{
	return time(t);
}
#endif

