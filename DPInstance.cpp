/*!
	@file DPInstance.cpp
	@brief DirectPlay reimplementation instance
	@author Arves100
	@date 05/02/2022
*/
#include "StdAfx.h"
#include "DPInstance.h"
#include "Globals.h"
#include "DPMsg.h"

#define LOG(fmt, ...) MessageBoxA(Globals::Get()->GameWindow, #fmt, "", 0);
#define ENET_BUFFER_SIZE 1024
#define FURFIGHTERS_PORT 55870

enum ENetChannels
{
	ENET_CHANNEL_NORMAL,
	ENET_CHANNEL_CHAT,
	ENET_CHANNEL_MAX,
};

DPInstance::DPInstance(void)
{
	m_pHost = nullptr;
	m_szGameName = "";
	m_bHost = false;
	m_bService = false;
	m_bConnected = false;
	m_pClientPeer = nullptr;
}

DPInstance::~DPInstance(void)
{
	m_bService = false;

	if (m_thread.joinable())
		m_thread.join();

	if (m_pHost)
	{
		if (m_bHost)
		{ // SERVER
			for (const auto& p : m_vPlayers)
			{
				p.second->ForceDisconnect();
				enet_peer_reset(p.second->GetPeer());
			}
		}
		else
		{ // CLIENT
			if (m_pClientPeer)
				enet_peer_reset(m_pClientPeer);
		}

		m_vPlayers.clear();
		enet_host_destroy(m_pHost);
	}
}

HRESULT DPInstance::AddPlayerToGroup(DPID idGroup, DPID idPlayer)
{
	LOG(AddPlayerToGroup);
	return DP_OK;
}

HRESULT DPInstance::EnumSessions(LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumSessionsCallback2, LPVOID lpContext, DWORD dwFlags)
{
	LOG(EnumSessions);
	return DP_OK;
}

HRESULT DPInstance::GetCaps(LPDPCAPS lpDPCaps, DWORD dwFlags)
{
	LOG(GetCaps);
	return DP_OK;
}

HRESULT DPInstance::SendEx(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, LPDWORD lpdwMsgID)
{
	//LOG(SendEx);
	return DP_OK;
}

HRESULT DPInstance::SendChatMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage)
{
	LOG(SendChatMessage);
	return DP_OK;
}

HRESULT DPInstance::SetSessionDesc(LPDPSESSIONDESC2 lpSessDesc, DWORD dwFlags)
{
	LOG(SetSessionDesc);
	return DP_OK;
}

HRESULT DPInstance::Send(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize)
{
	//LOG(Send);
	return DP_OK;
}

HRESULT DPInstance::CreateGroup(LPDPID lpidGroup, LPDPNAME lpGroupName, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	LOG(CreateGroup);
	return DP_OK;
}

HRESULT DPInstance::Receive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize)
{
	if (dwFlags == 0)
		dwFlags = DPRECEIVE_ALL;

	Service(0); // Service enet then dispatch the messages

	for (auto it = m_vMessages.begin(); it != m_vMessages.end(); it++)
	{
		bool canProcess = true;

		if (dwFlags & DPRECEIVE_TOPLAYER)
		{
			if (it->GetTo() != *lpidTo)
				canProcess = false;
		}

		if (dwFlags & DPRECEIVE_FROMPLAYER)
		{
			if (it->GetFrom() != *lpidFrom)
				canProcess = false;
		}

		if (!canProcess)
			continue;

		auto ret = it->FixSysMessage(lpData, lpdwDataSize);

		if (ret != DP_OK)
			return ret;

		if (!(dwFlags & DPRECEIVE_PEEK))
			m_vMessages.erase(it);

		return DP_OK;
	}

	return DPERR_NOMESSAGES;
}

HRESULT DPInstance::DestroyPlayer(DPID idPlayer)
{
	LOG(DestroyPlayer);

	auto v = m_vPlayers.find(idPlayer);

	if (v == m_vPlayers.end())
		return DPERR_INVALIDPLAYER;

	m_vPlayers.erase(v);

	if (m_bHost && v->second->IsHostMade()) // Tell all the other players that a player disconnected
		enet_host_broadcast(m_pHost, ENET_CHANNEL_CHAT, DPMsg::DestroyPlayer(v->second));

	v->second->Disconnect();

	return DP_OK;
}

HRESULT DPInstance::CreatePlayer(LPDPID lpidPlayer, LPDPNAME lpPlayerName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	LOG(CreatePlayer);

	if (!m_bHost)
	{
		if (dwFlags & DPPLAYER_SERVERPLAYER)
			return DPERR_CANTCREATEPLAYER;

		if (!m_bConnected)
			return DPERR_NOCONNECTION;
	}

	if (m_pHost)
		*lpidPlayer = (DWORD)m_vPlayers.size() + 1;
	else
	{ // CLIENT: Ask the network for a new player id
		enet_peer_send(m_pClientPeer, ENET_CHANNEL_NORMAL, DPMsg::CallNewId());

		while (true) // Idle until we receive the new id
		{
			Service(0);

			if (!m_bConnected)
				return DPERR_CONNECTIONLOST; // F

			if (m_pClientPeer->data != 0)
				break;
		}
	}

	const auto player = std::make_shared<DPPlayer>();
	player->Create(*lpidPlayer, lpPlayerName->lpszShortNameA, lpPlayerName->lpszLongNameA, hEvent, lpData, dwDataSize, dwFlags & DPPLAYER_SPECTATOR, dwFlags & DPPLAYER_SERVERPLAYER);

	if (m_bHost && player->IsHostMade())
	{
		// Tell all the other peers that a new player is online
		enet_host_broadcast(m_pHost, ENET_CHANNEL_CHAT, DPMsg::NewPlayer(player, (DWORD)m_vPlayers.size()));
	}

	m_vPlayers.insert_or_assign(*lpidPlayer, player); // Add it to our local player list

	return DP_OK;
}

void DPInstance::Service(uint32_t timeout)
{
	ENetEvent evt;
	if (enet_host_service(m_pHost, &evt, timeout))
	{
		switch (evt.type)
		{
		case ENET_EVENT_TYPE_DISCONNECT:
		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
			if (!m_bHost)
			{ // CLIENT
				enet_peer_reset(m_pClientPeer);
				m_pClientPeer = nullptr;
				m_bConnected = false;

				DPMSG_SESSIONLOST msg2;
				msg2.dwType = DPSYS_SESSIONLOST;

				DPMsg msg(0, 0, DPMSG_TYPE_SYSTEM);
				msg.AddToSerialize(&msg2);

				// Remove all messages and push the session lost one, telling the app the we lost the connection				m_vMessages.clear();
				m_vMessages.push_back(msg);
			}
			else
			{ // SERVER
				if (evt.peer->data) // authenticated player
				{
					auto id = (DPID)evt.peer->data;
					auto it = m_vPlayers.find(id);

					if (it == m_vPlayers.end())
						break; // how?

					// tell all the peers that a player disconnected

					m_vPlayers.erase(it);
					enet_host_broadcast(m_pHost, ENET_CHANNEL_CHAT, DPMsg::DestroyPlayer(it->second));
				}

				break;
			}

			break;

		case ENET_EVENT_TYPE_CONNECT:
			if (!m_bHost)
				m_bConnected = true;
			else if (evt.data == 0)
			{
				// SERVER: Send game info to client then kick it
				enet_peer_send(evt.peer, ENET_CHANNEL_NORMAL, DPMsg::CreateRoomInfo(m_gSession, m_dwMaxPlayers, m_vPlayers.size(), m_szGameName.c_str(), m_adwUser));
				enet_peer_disconnect_later(evt.peer, 0);
			}

			break;

		case ENET_EVENT_TYPE_RECEIVE:
		{
			DPMsg msg(evt.packet, true);

			if (m_bHost)
			{
				// Setup peer id and send it back
				if (msg.GetType() == DPMSG_TYPE_CALL_NEWID)
				{ 
					DPID id = (DPID)m_vPlayers.size() + 1;
					enet_peer_send(evt.peer, ENET_CHANNEL_NORMAL, DPMsg::NewId(id));
					evt.peer->data = (LPVOID)id; // set id which means the player is authenticated

					break; // Do not add this internal message to the queue
				}
			}
			else
			{
				if (msg.GetType() == DPMSG_TYPE_NEWID)
				{
					m_pClientPeer->data = (LPVOID)msg.Read2(sizeof(DPID)); // Assign the readed id
					break; // Do not add this internal message to the queue
				}
			}

			if (evt.peer->data != 0)
			{
				auto p = m_vPlayers[(DPID)evt.peer->data];
				if (msg.GetTo() == p->GetId())
					p->FireEvent(); // Fire handle event as specified by DirectPlay
			}

			m_vMessages.push_back(msg);
			break;
		}
		}
	}
}

HRESULT DPInstance::SetPlayerData(DPID idPlayer, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	LOG(SetPlayerData);
	return TRUE;
}

HRESULT DPInstance::Open(LPDPSESSIONDESC2 lpsd, DWORD dwFlags)
{
	if (lpsd->dwFlags & DPCAPS_ASYNCSUPPORTED)
		return DPERR_UNAVAILABLE; // No

	if (dwFlags == DPOPEN_CREATE)
	{
		if (m_pHost)
			return DPERR_ALREADYINITIALIZED;

		ENetAddress addr;
		addr.ipv6 = ENET_HOST_ANY;
		addr.port = FURFIGHTERS_PORT;

		if (FAILED(CoCreateGuid(&m_gSession)))
			return DPERR_CANNOTCREATESERVER;

		m_pHost = enet_host_create(&addr, lpsd->dwMaxPlayers, ENET_CHANNEL_MAX, 0, 0, ENET_BUFFER_SIZE);

		if (!m_pHost)
			return DPERR_CANNOTCREATESERVER;

		m_bHost = true;
			
	}
	else if (dwFlags == DPOPEN_JOIN)
	{
		m_bHost = false;

		if (m_pClientPeer)
		{
			enet_peer_disconnect(m_pClientPeer, 0);
			Service(1000);
			enet_peer_reset(m_pClientPeer);
			m_pClientPeer = nullptr;
		}

		auto it = m_vEnumAddr.find(lpsd->guidInstance);

		if (it == m_vEnumAddr.end())
			return DPERR_INVALIDPARAMS;

		m_pClientPeer = enet_host_connect(m_pHost, &it->second, ENET_CHANNEL_MAX, 1);
		//return DPERR_CONNECTING;
		Service(5000);

		if (!m_bConnected)
		{
			enet_peer_reset(m_pClientPeer);
			m_pClientPeer = nullptr;
			return DPERR_NOCONNECTION;
		}
	}

	return DP_OK;
}

HRESULT DPInstance::Close(void)
{
	m_bService = false;

	if (m_thread.joinable())
		m_thread.join();

	if (m_pHost)
	{
		if (m_bHost)
		{ // SERVER
			for (const auto& dp : m_vPlayers)
			{
				dp.second->Disconnect();
			}
		}
		else
		{ // CLIENT
			enet_peer_disconnect(m_pClientPeer, 0);
		}

		Service(5000); // let everything disconnect gracefully

		if (m_pClientPeer) // CLIENT: reset peer
		{
			enet_peer_reset(m_pClientPeer);
			m_pClientPeer = nullptr;
		}

		m_vPlayers.clear();

		enet_host_destroy(m_pHost);
		m_pHost = nullptr;
	}

	m_bConnected = false;
	m_bHost = false;

	return DP_OK;
}

HRESULT DPInstance::InitializeConnection(LPVOID lpConnection, DWORD dwFlags)
{
	if (!lpConnection)
		return DPERR_INVALIDPARAMS; // We do not support discovery

	LPDPADDRESS addr = (LPDPADDRESS)lpConnection;

	if (lpConnection)
	{
		if (m_pHost)
			return DPERR_ALREADYINITIALIZED;

		// Client needs host created immidiatly so we can connect and query game info
		m_pHost = enet_host_create(nullptr, 1, ENET_CHANNEL_MAX, 0, 0, ENET_BUFFER_SIZE);

		if (!m_pHost)
			return DPERR_UNINITIALIZED;
	}

	return DP_OK;
}

bool DPInstance::GetAddressFromDPAddress(LPVOID lpConnection, ENetAddress* out)
{
	LPBYTE b = (LPBYTE)lpConnection;
	LPDPADDRESS addr = (LPDPADDRESS)lpConnection;
	b += sizeof(DPADDRESS);
	bool setIp = false;

	if (InlineIsEqualGUID(addr->guidDataType, DPAID_TotalSize))
	{
		DWORD sz = *(DWORD*)b;
		b += sizeof(DWORD);

		for (size_t i = 0; i < sz;)
		{
			LPDPADDRESS addr2 = (LPDPADDRESS)(b + i);

			if (InlineIsEqualGUID(addr2->guidDataType, DPAID_INet))
			{
				char ip[20];
				memcpy(ip, b + i + sizeof(DPADDRESS), addr2->dwDataSize);
				enet_address_set_ip(out, ip);
				setIp = true;
			}
			else if (InlineIsEqualGUID(addr2->guidDataType, DPAID_INetPort))
			{
				out->port = *(USHORT*)(b + i + sizeof(DPADDRESS));
			}

			i += sizeof(DPADDRESS) + addr2->dwDataSize;
		}
	}

	return setIp;
}

HRESULT DPInstance::EnumConnections(LPCGUID lpguidApplication, LPDPENUMCONNECTIONSCALLBACK lpEnumCallback, LPVOID lpContext, DWORD dwFlags)
{
	DPNAME dpName;
	dpName.dwSize = sizeof(dpName);
	dpName.dwFlags = 0;
	dpName.lpszShortNameA = (LPSTR)"ENet";
	dpName.lpszLongNameA = (LPSTR)"ENet Network Provider";

	lpEnumCallback(&GUID_ENet, nullptr, 0, &dpName, 0, lpContext);

	return DP_OK;
}

HRESULT DPInstance::GetSessionDesc(LPVOID lpData, LPDWORD lpdwDataSize)
{
	LOG(GetSessionDesc);
	return TRUE;
}

HRESULT DPInstance::GetPlayerData(DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags)
{
	LOG(GetPlayerData);
	return TRUE;
}
