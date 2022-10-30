/*!
	@file DPInstanceJoin.cpp
	@brief DirectPlay joiner
	@author Arves100
	@date 04/07/2022
*/
#include "StdAfx.h"
#include "DPInstanceJoin.h"

HRESULT DPInstanceJoin::Send(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize)
{
	if (!m_peer)
		return DPERR_NOCONNECTION;

	if (!m_bAuthenticated)
		return DPERR_NOTLOGGEDIN;

	auto msg = MakeMessage(idFrom, idTo, dwFlags, lpData, dwDataSize);

	return enet_peer_send(m_peer, ENET_CHANNEL_NORMAL, msg.Serialize((dwFlags & DPSEND_GUARANTEED) ? ENET_PACKET_FLAG_RELIABLE : 0)) == 0 ? DP_OK : DPERR_BUSY;
}

HRESULT DPInstanceJoin::SendChatMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage)
{
	if (!m_peer)
		return DPERR_CONNECTIONLOST;

	if (!m_bAuthenticated)
		return DPERR_NOTLOGGEDIN;

	// @note: the host acts as a broadcaster to other peers !
	return enet_peer_send(m_peer, ENET_CHANNEL_CHAT, DPMsg::ChatPacket(idFrom, idTo, lpChatMessage, dwFlags & DPSEND_GUARANTEED)) == 0 ? DP_OK : DPERR_BUSY;
}

HRESULT DPInstanceJoin::Receive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize)
{

}

HRESULT DPInstanceJoin::Close()
{
	if (m_pHost)
	{
		if (m_peer)
			enet_peer_disconnect(m_peer, 0);
	}

	return DPInstance::Close();
}
