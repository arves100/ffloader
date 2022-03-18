/*!
	@file FakeDP.h
	@brief NetLib FakeDP class definition
	@author Arves100
	@date 01/11/2020
*/
#pragma once

#ifndef HRESULT_INT
#define HRESULT_INT int  // typedef edit for integration with decomp
#endif

class DPInstance;  // edit for integration with decomp

class DLLAPI FakeDP
{
public:
	FakeDP(void);
	virtual ~FakeDP(void);

	// NetLib definitions

	HRESULT_INT AddPlayerToGroup(DPID idGroup, DPID idPlayer);
	HRESULT_INT EnumSessions(LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumSessionsCallback2, LPVOID lpContext, DWORD dwFlags);
	HRESULT_INT GetCaps(LPDPCAPS lpDPCaps, DWORD dwFlags);
	HRESULT_INT SendEx(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, LPDWORD lpdwMsgID);
	HRESULT_INT SendChatMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage);
	HRESULT_INT SetSessionDesc(LPDPSESSIONDESC2 lpSessDesc, DWORD dwFlags);
	HRESULT_INT Send(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize);
	HRESULT_INT CreateGroup(LPDPID lpidGroup, LPDPNAME lpGroupName, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags);
	HRESULT_INT Receive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize);
	HRESULT_INT DestroyPlayer(DPID idPlayer);
	HRESULT_INT CreatePlayer(LPDPID lpidPlayer, LPDPNAME lpPlayerName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags);
	HRESULT_INT SetPlayerData(DPID idPlayer, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags);
	HRESULT_INT Open(LPDPSESSIONDESC2 lpsd, DWORD dwFlags);
	HRESULT_INT Close(void);
	HRESULT_INT InitializeConnection(LPVOID lpConnection, DWORD dwFlags);
	virtual HRESULT_INT Release(void);
	HRESULT_INT EnumConnections(LPCGUID lpguidApplication, LPDPENUMCONNECTIONSCALLBACK lpEnumCallback, LPVOID lpContext, DWORD dwFlags);
	HRESULT_INT GetSessionDesc(LPVOID lpData, LPDWORD lpdwDataSize);
	HRESULT_INT GetPlayerData(DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags);

private:
	DPInstance* m_dp;
};
