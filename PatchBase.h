/*!
	@author Arves100
	@date 06/02/2022
	@file PatchBase.h
	@brief Base patch structure information
*/
#pragma once

#include <stdint.h>

struct PatchBytes
{
	uint32_t offset;
	uint8_t original;
	uint8_t patched;
};

struct PatchArray
{
	const PatchBytes* data;
	size_t len;
};

static bool ApplyPatchBytes(const PatchBytes& pb)
{
	SIZE_T readed;
	uint8_t check;
	auto pos = (LPBYTE)(Globals::Get()->BaseAddress) + pb.offset;

	if (!ReadProcessMemory(Globals::Get()->GameProcess, pos, &check, sizeof(check), &readed))
		return false;

	if (readed != sizeof(check) || check != pb.original)
		return false;

	if (!WriteProcessMemory(Globals::Get()->GameProcess, pos, &pb.patched, sizeof(pb.patched), &readed))
		return false;

	return readed == sizeof(check);
}

static bool ApplyPatchArray(const PatchArray& a)
{
	for (size_t i = 0; i < a.len; i++)
	{
		if (!ApplyPatchBytes(a.data[i]))
			return false;
	}

	return true;
}

inline void ApplyPatch(const PatchArray* a, size_t len, const wchar_t* name)
{
	for (size_t i = 0; i < len; i++)
	{
		if (!ApplyPatchArray(a[i]))
		{
			std::wstring err = L"Unable to apply patch: ";
			err += name;
			FATAL2(err.c_str());
		}
	}
}

#define WINDOWED_MODE_POSITION 0x0060FE2C
#define SUPPORTED_SCREEN_MODE_POSITION 0x005B1DA8U
#define IDA_BASE 0x400000U
