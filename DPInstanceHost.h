/*!
	@file DPInstanceHost.h
	@brief DirectPlay main host
	@author Arves100
	@date 04/07/2022
*/
#pragma once

#include "DPInstance.h"

class DPInstanceHost final : public DPInstance
{
public:
	HRESULT SendChatMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage);
	HRESULT GetCaps(LPDPCAPS lpDPCaps, DWORD dwFlags) override;
	HRESULT Send(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize) override;
	HRESULT Receive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize) override;
	HRESULT Close() override;

private:
	HRESULT PacketToPlayer(uint8_t channel, ENetPacket* pk);

	/// Maximum players that can be accepted
	DWORD m_dwMaxPlayer;

	/// ?
	DWORD m_dwFlags;

	/// Logged players
	std::unordered_map<DPID, std::shared_ptr<DPPlayer>> m_vPlayers;
};
