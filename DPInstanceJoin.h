/*!
	@file DPInstanceJoin.h
	@brief DirectPlay joiner
	@author Arves100
	@date 05/02/2022
*/
#pragma once

#include "DPInstance.h"

class DPInstanceJoin final : public DPInstance
{
public:
	HRESULT SendChatMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage) override;
	HRESULT Send(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize) override;
	HRESULT Receive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize) override;
	HRESULT Close() override;

private:
	ENetPeer* m_peer;

	/// If we properly received an ID or not
	bool m_bAuthenticated;
};
