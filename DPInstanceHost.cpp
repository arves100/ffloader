/*!
	@file DPInstanceHost.cpp
	@brief DirectPlay main host
	@author Arves100
	@date 04/07/2022
*/
#include "StdAfx.h"
#include "DPInstanceHost.h"

HRESULT DPInstanceHost::SendChatMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage)
{
	const auto msg = DPMsg::ChatPacket(idFrom, idTo, lpChatMessage, dwFlags & DPSEND_GUARANTEED);

	xlog(DPINSTANCEHOST, DEBUG2, "Send chat message %s to %u", lpChatMessage->lpszMessageA, idTo);

	if (idTo != DPID_ALLPLAYERS)
		return PacketToPlayer(ENET_CHANNEL_CHAT, msg);
	
	enet_host_broadcast(m_pHost, ENET_CHANNEL_CHAT, msg);
}

HRESULT DPInstanceHost::GetCaps(LPDPCAPS lpDPCaps, DWORD dwFlags)
{
	auto hr = DPInstance::GetCaps(lpDPCaps, dwFlags);
	lpDPCaps->dwFlags |= DPCAPS_ISHOST;
	return hr;
}

HRESULT DPInstanceHost::Send(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize)
{
	auto msg = MakeMessage(idFrom, idTo, dwFlags, lpData, dwDataSize);

	if (idTo == DPID_ALLPLAYERS)
	{
		auto pk = msg.Serialize((dwFlags & DPSEND_GUARANTEED) ? ENET_PACKET_FLAG_RELIABLE : 0);
		enet_host_broadcast(m_pHost, ENET_CHANNEL_NORMAL, pk);
	}
	else if (idTo != m_cMyself->GetId())
	{
		auto pk = msg.Serialize((dwFlags & DPSEND_GUARANTEED) ? ENET_PACKET_FLAG_RELIABLE : 0);
		return PacketToPlayer(ENET_CHANNEL_NORMAL, pk);
	}
	else
	{
		// Send message to ourselves
		PushMessage(msg);
	}

	return DP_OK;
}

HRESULT DPInstanceHost::Close()
{
	if (m_pHost)
	{
		for (const auto& dp : m_vPlayers)
		{
			dp.second->Disconnect();
		}
	}

	m_vPlayers.clear();

	return DPInstance::Close();
}

HRESULT DPInstanceHost::Receive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize)
{

}

HRESULT DPInstanceHost::CreatePlayer(LPDPID lpidPlayer, LPDPNAME lpPlayerName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	if (!m_pHost)
		return DPERR_CANTCREATEPLAYER;

	*lpidPlayer = (DWORD)m_vPlayers.size() + 1;
	xlog(DPINSTANCEHOST, DEBUG, "New player created %u", *lpidPlayer);

	const auto player = std::make_shared<DPPlayer>();
	player->Create(*lpidPlayer, lpPlayerName->lpszShortNameA, lpPlayerName->lpszLongNameA, hEvent, lpData, dwDataSize, dwFlags & DPPLAYER_SPECTATOR, dwFlags & DPPLAYER_SERVERPLAYER);
	
	if (player->IsHostMade())
	{
		// Tell all the other peers that a new player is online
		enet_host_broadcast(m_pHost, ENET_CHANNEL_CHAT, DPMsg::NewPlayer(player, (DWORD)m_vPlayers.size()));
	}

	m_vPlayers.insert_or_assign(*lpidPlayer, player); // Add it to our local player list
	return DP_OK;
}