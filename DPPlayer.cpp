/*!
	@author Arves100
	@brief DirectPlay player rapresentation
	@date 07/02/2022
	@file DPPlayer.cpp
*/
#include "stdafx.h"
#include "DPPlayer.h"

DPPlayer::DPPlayer() : m_dwId(0), m_hEvent(INVALID_HANDLE_VALUE), m_lpData(nullptr), m_dwDataSize(0), m_bIsSpectator(false), m_bMadeByHost(false), m_lpRemoteData(nullptr), m_dwRemoteDataSize(0), m_pPeer(nullptr) {}
DPPlayer::~DPPlayer()
{
	SetLocalData(nullptr, 0);
	SetRemoteData(nullptr, 0);
}

void DPPlayer::Create(DPID id, const char* shortName, const char* longName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, bool spectator, bool madeByHost)
{
	m_dwId = id;

	if (longName)
		m_szLongName = longName;
	
	if (shortName)
		m_szShortName = shortName;
	m_hEvent = hEvent;
	SetLocalData(lpData, dwDataSize);
	m_dwDataSize = dwDataSize;
	m_bIsSpectator = spectator;
	m_bMadeByHost = madeByHost;
}

void DPPlayer::FireEvent()
{
	SetEvent(m_hEvent);
}

void DPPlayer::Disconnect()
{
	if (!m_pPeer)
		return;

	enet_peer_disconnect(m_pPeer, 0);
	m_pPeer = nullptr;
}

void DPPlayer::ForceDisconnect()
{
	if (!m_pPeer)
		return;

	enet_peer_disconnect_now(m_pPeer, 0);
	m_pPeer = nullptr;
}
