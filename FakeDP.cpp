/*!
	@file FakeDP.h
	@brief NetLib FakeDP class definition
	@author Arves100
	@date 01/11/2020
*/
#include "StdAfx.h"
#include "FakeDP.h"
#include "DPInstance.h"

FakeDP::FakeDP(void) : m_dp(nullptr)
{
	m_dp = new DPInstance();
}

FakeDP::~FakeDP(void)
{
	if (m_dp)
	{
		delete m_dp;
		m_dp = nullptr;
	}
}

HRESULT_INT FakeDP::AddPlayerToGroup(DPID idGroup, DPID idPlayer)
{
	return m_dp->AddPlayerToGroup(idGroup, idPlayer);
}

HRESULT_INT FakeDP::EnumSessions(LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumSessionsCallback2, LPVOID lpContext, DWORD dwFlags)
{
	return m_dp->EnumSessions(lpsd, dwTimeout, lpEnumSessionsCallback2, lpContext, dwFlags);
}

HRESULT_INT FakeDP::GetCaps(LPDPCAPS lpDPCaps, DWORD dwFlags)
{
	return m_dp->GetCaps(lpDPCaps, dwFlags);
}

HRESULT_INT FakeDP::SendEx(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, LPDWORD lpdwMsgID)
{
	return m_dp->SendEx(idFrom, idTo, dwFlags, lpData, dwDataSize, dwPriority, dwTimeout, lpContext, lpdwMsgID);
}

HRESULT_INT FakeDP::SendChatMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage)
{
	return m_dp->SendChatMessage(idFrom, idTo, dwFlags, lpChatMessage);
}

HRESULT_INT FakeDP::SetSessionDesc(LPDPSESSIONDESC2 lpSessDesc, DWORD dwFlags)
{
	return m_dp->SetSessionDesc(lpSessDesc, dwFlags);
}

HRESULT_INT FakeDP::Send(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize)
{
	return m_dp->Send(idFrom, idTo, dwFlags, lpData, dwDataSize);
}

HRESULT_INT FakeDP::CreateGroup(LPDPID lpidGroup, LPDPNAME lpGroupName, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	return m_dp->CreateGroup(lpidGroup, lpGroupName, lpData, dwDataSize, dwFlags);
}

HRESULT_INT FakeDP::Receive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize)
{
	return m_dp->Receive(lpidFrom, lpidTo, dwFlags, lpData, lpdwDataSize);
}

HRESULT_INT FakeDP::DestroyPlayer(DPID idPlayer)
{
	return m_dp->DestroyPlayer(idPlayer);
}

HRESULT_INT FakeDP::CreatePlayer(LPDPID lpidPlayer, LPDPNAME lpPlayerName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	return m_dp->CreatePlayer(lpidPlayer, lpPlayerName, hEvent, lpData, dwDataSize, dwFlags);
}

HRESULT_INT FakeDP::SetPlayerData(DPID idPlayer, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	return m_dp->SetPlayerData(idPlayer, lpData, dwDataSize, dwFlags);
}

HRESULT_INT FakeDP::Open(LPDPSESSIONDESC2 lpsd, DWORD dwFlags)
{
	return m_dp->Open(lpsd, dwFlags);
}

HRESULT_INT FakeDP::Close(void)
{
	return m_dp->Close();
}

HRESULT_INT FakeDP::InitializeConnection(LPVOID lpConnection, DWORD dwFlags)
{
	return m_dp->InitializeConnection(lpConnection, dwFlags);
}

HRESULT_INT FakeDP::Release(void)
{
	delete m_dp;
	m_dp = nullptr;
	return 0;
}

HRESULT_INT FakeDP::EnumConnections(LPCGUID lpguidApplication, LPDPENUMCONNECTIONSCALLBACK lpEnumCallback, LPVOID lpContext, DWORD dwFlags)
{
	return m_dp->EnumConnections(lpguidApplication, lpEnumCallback, lpContext, dwFlags);
}

HRESULT_INT FakeDP::GetSessionDesc(LPVOID lpData, LPDWORD lpdwDataSize)
{
	return m_dp->GetSessionDesc(lpData, lpdwDataSize);
}

HRESULT_INT FakeDP::GetPlayerData(DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags)
{
	return m_dp->GetPlayerData(idPlayer, lpData, lpdwDataSize, dwFlags);
}
