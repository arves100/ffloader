/*!
	@file Xlog.h
	@brief Simple logging stuff
	@author Arves100
	@date 04/07/2022
*/
#pragma once

#include <stdint.h>

enum ELogLevels
{
	INFO,
	WARN,
	ERRO,
	DEBUG,
	DEBUG2,
	NET,
};

enum ELogChannels
{
	DPINSTANCE,
	DPMSG,
	DPINSTANCEHOST,
	DPINSTANCEJOIN,
};

extern void xlog(uint8_t ch, uint8_t lvl, const char* fmt, ...);
