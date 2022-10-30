/*!
	@file Xlog.cpp
	@brief Simple logging stuff
	@author Arves100
	@date 04/07/2022
*/
#include "StdAfx.h"
#include "Xlog.h"

static uint8_t g_currentLvl;

static const char* GetChannel(uint8_t ch)
{
	switch (ch)
	{
	case DPINSTANCE:
		return "DPInstance";
		break;
	default:
		break;
	}

	return "?";
}

static const char* GetLevel(uint8_t lv)
{
	switch (lv)
	{
	case INFO:
		return "INFO";
	case WARN:
		return "WARN";
	case ERRO:
		return "ERRO";
	case DEBUG:
		return "DEBUG";
	case DEBUG2:
		return "DEBUGEX";
	case NET:
		return "NETDBG";
	default:
		break;
	}

	return "?";
}

void xlog(uint8_t ch, uint8_t lvl, const char* fmt, ...)
{
	va_list vl;

	if (lvl > g_currentLvl)
		return;

	printf("[%s] [%s] >> ", GetChannel(ch), GetLevel(ch));
	va_start(fmt, vl);
	vprintf(fmt, vl);
	va_end(vl);
	printf("\n");
}
