/*!
	@file FakeDP.h
	@brief NetLib FakeDP class definition
	@author Arves100
	@date 01/11/2020
*/
#include "StdAfx.h"
#include "FakeDP.h"

#define LOG(fmt, ...)

#define DEFINE_GUID2(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUID2(DPSPGUID_TCPIP, 0x36E95EE0, 0x8577, 0x11cf, 0x96, 0xc, 0x0, 0x80, 0xc7, 0x53, 0x4e, 0x82);
DEFINE_GUID2(DPSPGUID_MODEM, 0x44eaa760, 0xcb68, 0x11cf, 0x9c, 0x4e, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

FakeDP::FakeDP(void)
{
	LOG(FakeDP costructor);
}

FakeDP::~FakeDP(void)
{
	LOG(FakeDP decostructor);
}

HRESULT_INT FakeDP::AddPlayerToGroup(DPID idGroup, DPID idPlayer)
{
	LOG(AddPlayerToGroup);
	return TRUE;
}

HRESULT_INT FakeDP::EnumSessions(LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumSessionsCallback2, LPVOID lpContext, DWORD dwFlags)
{
	LOG(EnumSessions);
	return TRUE;
}

HRESULT_INT FakeDP::GetCaps(LPDPCAPS lpDPCaps, DWORD dwFlags)
{
	LOG(GetCaps);
	return TRUE;
}

HRESULT_INT FakeDP::SendEx(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, LPDWORD lpdwMsgID)
{
	LOG(SendEx);
	return TRUE;
}

HRESULT_INT FakeDP::SendChatMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage)
{
	LOG(SendChatMessage);
	return TRUE;
}

HRESULT_INT FakeDP::SetSessionDesc(LPDPSESSIONDESC2 lpSessDesc, DWORD dwFlags)
{
	LOG(SetSessionDesc);
	return TRUE;
}

HRESULT_INT FakeDP::Send(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize)
{
	LOG(Send);
	return TRUE;
}

HRESULT_INT FakeDP::CreateGroup(LPDPID lpidGroup, LPDPNAME lpGroupName, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	LOG(CreateGroup);
	return TRUE;
}

HRESULT_INT FakeDP::Receive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize)
{
	LOG(Receive);
	return TRUE;
}

HRESULT_INT FakeDP::DestroyPlayer(DPID idPlayer)
{
	LOG(DestroyPlayer);
	return TRUE;
}

HRESULT_INT FakeDP::CreatePlayer(LPDPID lpidPlayer, LPDPNAME lpPlayerName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	LOG(CreatePlayer);
	return TRUE;
}

HRESULT_INT FakeDP::SetPlayerData(DPID idPlayer, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	LOG(SetPlayerData);
	return TRUE;
}

HRESULT_INT FakeDP::Open(LPDPSESSIONDESC2 lpsd, DWORD dwFlags)
{
	LOG(Open);
	return TRUE;
}

HRESULT_INT FakeDP::Close(void)
{
	LOG(Close);
	return TRUE;
}

HRESULT_INT FakeDP::InitializeConnection(LPVOID lpConnection, DWORD dwFlags)
{
	//m_pPeer = RakPeerInterface::GetInstance();

	LOG(InitializeConnection %S, (const char*)lpConnection);
	return TRUE;
}

HRESULT_INT FakeDP::Release(void)
{
	LOG(Release);
	return TRUE;
}

HRESULT_INT FakeDP::EnumConnections(LPCGUID lpguidApplication, LPDPENUMCONNECTIONSCALLBACK lpEnumCallback, LPVOID lpContext, DWORD dwFlags)
{
	LOG(EnumConnections GUID: %d-%d-%d-%d%d%d%d%d%d%d%d flags %lu, lpguidApplication->Data1, lpguidApplication->Data2, lpguidApplication->Data3, lpguidApplication->Data4[0],
		lpguidApplication->Data4[1], lpguidApplication->Data4[2], lpguidApplication->Data4[3], lpguidApplication->Data4[4], lpguidApplication->Data4[5], lpguidApplication->Data4[6],
		lpguidApplication->Data4[7], dwFlags);

	DPNAME dpName;
	dpName.dwFlags = 0;
	dpName.dwSize = sizeof(dpName);
	dpName.lpszLongNameA = (LPSTR)"a";
	dpName.lpszShortNameA = (LPSTR)"a";
	GUID g = DPSPGUID_TCPIP;

	lpEnumCallback(&g, (LPVOID)"127.0.0.1", 10, &dpName, 0, nullptr);

	g = DPSPGUID_MODEM;
	lpEnumCallback(&g, (LPVOID)"127.0.0.1", 10, &dpName, 0, nullptr);

	return TRUE;
}

HRESULT_INT FakeDP::GetSessionDesc(LPVOID lpData, LPDWORD lpdwDataSize)
{
	LOG(GetSessionDesc);
	return TRUE;
}

HRESULT_INT FakeDP::GetPlayerData(DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags)
{
	LOG(GetPlayerData);
	return TRUE;
}
