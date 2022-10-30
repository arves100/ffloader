/*!
	@file DPInstance.h
	@brief DirectPlay reimplementation instance
	@author Arves100
	@date 05/02/2022
*/
#pragma once

#include "DPPlayer.h"
#include "DPMsg.h"

using QueueMsg = std::vector<std::shared_ptr<DPMsg>>;

class DPInstance
{
public:
	DPInstance(void);
	~DPInstance(void);

	HRESULT AddPlayerToGroup(DPID idGroup, DPID idPlayer);
	HRESULT EnumSessions(LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumSessionsCallback2, LPVOID lpContext, DWORD dwFlags);
	
	virtual HRESULT GetCaps(LPDPCAPS lpDPCaps, DWORD dwFlags);
	virtual HRESULT SendEx(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, LPDWORD lpdwMsgID);
	virtual HRESULT SendChatMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage) = 0;
	
	HRESULT SetSessionDesc(LPDPSESSIONDESC2 lpSessDesc, DWORD dwFlags);
	virtual HRESULT Send(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize) = 0;
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

protected:
	/*!
		Creates a new message
		@param idFrom ID where the message was sent
		@param idTo ID where the message should be sent
		@param dwFlags DirectPlay flags
		@param lpData Data to send
		@param dwDataSize Length of the size
		@return Newly created message
	*/
	DPMsg MakeMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize);

	/*!
		Push a message to the packet queue
		@param msg Message to push
	*/
	void PushMessage(DPMsg msg);

	/// ENet host
	ENetHost* m_pHost;

	/// Name of the game to host
	std::string m_szGameName;

	/// App GUID
	GUID m_guidApp;

	/// Current player
	std::shared_ptr<DPPlayer> m_cMyself;

private:
	/// Message packet queues
	QueueMsg m_vMessages;

	bool GetAddressFromDPAddress(LPVOID lpConnection, ENetAddress* addr);
	void Service(uint32_t time);
	void SetupThreadedService(bool infinite);
	HRESULT EnumSessionOut(LPDPENUMSESSIONSCALLBACK2 cb, LPVOID ctx);
	
	// Shared
	GUID m_gSession;
	DWORD m_adwUser[4];

	// Client
	bool m_bConnected;
	ENetPeer* m_pClientPeer;
	std::unordered_map<GUID, ENetAddress, GUIDHasher> m_vEnumAddr;
	ENetAddress m_eConnectAddr;
	

	/// ENet Thread
	std::thread m_thread;

	bool m_bService;
	ULONGLONG m_ullServiceTimeout;
	ULONGLONG m_ullStartService;
};
