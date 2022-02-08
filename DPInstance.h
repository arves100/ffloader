/*!
	@file DPInstance.h
	@brief DirectPlay reimplementation instance
	@author Arves100
	@date 05/02/2022
*/
#pragma once

#include "DPPlayer.h"
#include "DPMsg.h"

using QueueMsg = std::vector<DPMsg>;

class DPInstance final
{
public:
	DPInstance(void);
	~DPInstance(void);

	HRESULT AddPlayerToGroup(DPID idGroup, DPID idPlayer);
	HRESULT EnumSessions(LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumSessionsCallback2, LPVOID lpContext, DWORD dwFlags);
	HRESULT GetCaps(LPDPCAPS lpDPCaps, DWORD dwFlags);
	HRESULT SendEx(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, LPDWORD lpdwMsgID);
	HRESULT SendChatMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage);
	HRESULT SetSessionDesc(LPDPSESSIONDESC2 lpSessDesc, DWORD dwFlags);
	HRESULT Send(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize);
	HRESULT CreateGroup(LPDPID lpidGroup, LPDPNAME lpGroupName, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags);
	HRESULT Receive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize);
	HRESULT DestroyPlayer(DPID idPlayer);
	HRESULT CreatePlayer(LPDPID lpidPlayer, LPDPNAME lpPlayerName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags);
	HRESULT SetPlayerData(DPID idPlayer, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags);
	HRESULT Open(LPDPSESSIONDESC2 lpsd, DWORD dwFlags);
	HRESULT Close(void);
	HRESULT InitializeConnection(LPVOID lpConnection, DWORD dwFlags);
	HRESULT EnumConnections(LPCGUID lpguidApplication, LPDPENUMCONNECTIONSCALLBACK lpEnumCallback, LPVOID lpContext, DWORD dwFlags);
	HRESULT GetSessionDesc(LPVOID lpData, LPDWORD lpdwDataSize);
	HRESULT GetPlayerData(DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags);

private:
	bool GetAddressFromDPAddress(LPVOID lpConnection, ENetAddress* addr);
	void Service(uint32_t time);

	ENetHost* m_pHost;

	// Shared
	std::string m_szGameName;
	std::unordered_map<DPID, std::shared_ptr<DPPlayer>> m_vPlayers;
	bool m_bHost;
	GUID m_gSession;
	QueueMsg m_vMessages; // we need a queue due to how DPlay works...
	DWORD m_adwUser[4];

	// Server
	DWORD m_dwMaxPlayers;

	// Client
	bool m_bConnected;
	ENetPeer* m_pClientPeer;
	std::unordered_map<GUID, ENetAddress, GUIDHasher> m_vEnumAddr;

	// ENet Thread
	std::thread m_thread;
	bool m_bService;
};
