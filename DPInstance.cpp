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

#define ENET_BUFFER_SIZE 4096
#define FURFIGHTERS_PORT 24900U
#define ENET_SERVICE_TIME 1000

#define TIMEOUT1 32
#define TIMEOUT2 5000
#define TIMEOUT3 10000

DPInstance::DPInstance(void)
{
	m_pHost = nullptr;
	m_szGameName = "";
	m_bService = false;
	m_bConnected = false;
	m_pClientPeer = nullptr;

	xlog(DPINSTANCE, DEBUG2, "Enet initialization");

	enet_initialize();

}

DPInstance::~DPInstance(void)
{
	xlog(DPINSTANCE, DEBUG2, "Enet shutdown!!!!!");

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

	enet_deinitialize();
}

HRESULT DPInstance::AddPlayerToGroup(DPID idGroup, DPID idPlayer)
{
	xlog(DPINSTANCE, DEBUG, "STUB: AddPlayerToGroup (idGroup=%u, idPlayer=%u)", idGroup, idPlayer);
	return DP_OK;
}

HRESULT DPInstance::EnumSessions(LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumSessionsCallback2, LPVOID lpContext, DWORD dwFlags)
{
	char addr[40];
	enet_address_get_ip(&m_eConnectAddr, addr, 40);

	xlog(DPINSTANCE, DEBUG2, "Start session %s:%d", addr, m_eConnectAddr.port);

	m_pClientPeer = enet_host_connect(m_pHost, &m_eConnectAddr, ENET_CHANNEL_MAX, 0);

	if (!m_pClientPeer)
		return DPERR_INVALIDOBJECT;

	m_guidFF = lpsd->guidApplication;

	if (!m_bService)
	{
		xlog(DPINSTANCE, DEBUG, "Async session...");

		m_ullServiceTimeout = ENET_SERVICE_TIME;
		
		if (dwFlags & DPENUMSESSIONS_ASYNC)
		{
			SetupThreadedService(dwFlags & DPENUMSESSIONS_ASYNC);
			return DP_OK;
		}

		if (dwTimeout == 0)
			dwTimeout = ENET_SERVICE_TIME; // DEFAULT

		xlog(DPINSTANCE, DEBUG2, "Service session %d...", dwTimeout);

		Service(dwTimeout);

		if (!m_bConnected)
			return DPERR_NOCONNECTION;

		xlog(DPINSTANCE, DEBUG2, "Service session for %d to query connections...", dwTimeout);
		Service(dwTimeout);
	}
	else
	{
		if (dwFlags & DPENUMSESSIONS_STOPASYNC)
			return EnumSessionOut(lpEnumSessionsCallback2, lpContext);

		xlog(DPINSTANCE, DEBUG, "EnunSession called without STOPASYNC flag");
		return DPERR_INVALIDPARAMS;
	}

	return EnumSessionOut(lpEnumSessionsCallback2, lpContext);;
}

HRESULT DPInstance::GetCaps(LPDPCAPS lpDPCaps, DWORD dwFlags)
{
	lpDPCaps->dwSize = sizeof(DPCAPS);
	lpDPCaps->dwFlags = 0;
	lpDPCaps->dwMaxBufferSize = ENET_BUFFER_SIZE;
	lpDPCaps->dwMaxQueueSize = 0;
	lpDPCaps->dwMaxPlayers = m_dwMaxPlayers;
	lpDPCaps->dwHundredBaud = 24;
	lpDPCaps->dwLatency = 0;
	lpDPCaps->dwMaxLocalPlayers = m_dwMaxPlayers;
	lpDPCaps->dwTimeout = 0;
	return DP_OK;
}

HRESULT DPInstance::SendEx(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, LPDWORD lpdwMsgID)
{
	HRESULT hr = Send(idFrom, idTo, dwFlags, lpData, dwDataSize);

	if (FAILED(hr))
		return hr;

	if (dwFlags & DPSEND_ASYNC)
	{
		if (!(dwFlags & DPSEND_NOSENDCOMPLETEMSG))
		{
			if (lpdwMsgID)
				*lpdwMsgID = m_vMessages.size() + 1; // unused operation here

			auto msg = std::make_shared<DPMsg>(DPMsg::CreateSendComplete(idFrom, idTo, dwFlags, dwPriority, dwTimeout, lpContext, 0, DP_OK, 0), true);
			m_vMessages.push_back(msg); // Add internal msg
			return DPERR_PENDING;
		}
	}

	return hr;
}

HRESULT DPInstance::SetSessionDesc(LPDPSESSIONDESC2 lpSessDesc, DWORD dwFlags)
{
	xlog(DPINSTANCE, DEBUG2, "SetSessionDesc %u", dwFlags);
	return DP_OK;
}


HRESULT DPInstance::CreateGroup(LPDPID lpidGroup, LPDPNAME lpGroupName, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
#ifdef _DEBUG
	printf("[LOADER] STUB: CreateGroup %p %p %p %d\n", lpidGroup, lpGroupName, lpData, dwDataSize);
#endif
	return DP_OK;
}

HRESULT DPInstance::Receive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize)
{
	Service(0); // Service enet then dispatch the messages

	if (dwFlags == 0)
		dwFlags = DPRECEIVE_ALL;

	if (!m_pHost)
	{
		xlog(DPINSTANCE, DEBUG, "Host is invalid!");
		return DPERR_GENERIC;
	}

	if (m_vMessages.size() != 0)
		xlog(DPINSTANCE, DEBUG, "Servicing %zu msg", m_vMessages.size());

	for (const auto it2 = m_vMessages.begin(); it2 != m_vMessages.end(); it2++)
	{
		const auto it = *it2;

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

		DWORD lastSize = *lpdwDataSize;

		*lpidFrom = it->GetFrom();
		*lpidTo = it->GetTo();

		if (it->GetType() == DPMSG_TYPE_SYSTEM)
		{
			auto ret = it->FixSysMessage(lpData, lpdwDataSize);

			if (ret != DP_OK)
				return ret;

			*lpidFrom = 0;

			if (*(DWORD*)lpData == DPSYS_DESTROYPLAYERORGROUP) // Patch to add required data
			{
				LPDPMSG_DESTROYPLAYERORGROUP destroyMsg = (LPDPMSG_DESTROYPLAYERORGROUP)lpData;

				auto p = m_vPlayers.find(destroyMsg->dpId);
				if (p != m_vPlayers.end())
				{
					destroyMsg->lpLocalData = a->Store(p->second->GetLocalData(), p->second->GetLocalDataSize());
					destroyMsg->dwLocalDataSize = p->second->GetLocalDataSize();
					destroyMsg->lpRemoteData = a->Store(p->second->GetRemoteData(), p->second->GetRemoteDataSize());
					destroyMsg->dwRemoteDataSize = p->second->GetRemoteDataSize();

					m_vPlayers.erase(p); // bye bye,,...
				}
			}
			else if (*(DWORD*)lpData == DPSYS_CREATEPLAYERORGROUP && !m_bHost) // Add player to the current players
			{
				DPMSG_CREATEPLAYERORGROUP* msg = (DPMSG_CREATEPLAYERORGROUP*)lpData;

				auto newPlayer = std::make_shared<DPPlayer>();
				newPlayer->Create(msg->dpId, msg->dpnName.lpszShortNameA, msg->dpnName.lpszLongNameA, nullptr, nullptr, 0, false, false);
				m_vPlayers.insert_or_assign(newPlayer->GetId(), newPlayer);
			}
		}
		else if (it->GetType() == DPMSG_TYPE_NEWID)
		{
			if (!m_bHost)
			{
				m_pClientPeer->data = (LPVOID) * (DPID*)it->Read2(sizeof(DPID));

#ifdef _DEBUG
				printf("[LOADER] New peer id %d", (DPID)m_pClientPeer->data);
#endif
			}

			m_vMessages.erase(it2);
			return DPERR_NOMESSAGES; // no queue and rm this internal msg
		}
		else if (it->GetType() == DPMSG_TYPE_GAME)
		{
			*lpdwDataSize = *(DWORD*)it->Read2(sizeof(DWORD));

			if (lastSize < *lpdwDataSize)
				return DPERR_BUFFERTOOSMALL;

			memcpy_s(lpData, lastSize, it->Read2(*lpdwDataSize), *lpdwDataSize);

#ifdef _DEBUG
			printf("[LOADER] Dump ");
			for (DWORD m = 0; m < *lpdwDataSize; m++)
				printf("%x ", *(CHAR*)lpData + m);
			printf("\n");
#endif
		}

		if (!(dwFlags & DPRECEIVE_PEEK))
			m_vMessages.erase(it2);

		return DP_OK;
	}

	return DPERR_NOMESSAGES;
}

HRESULT DPInstance::DestroyPlayer(DPID idPlayer)
{
	auto v = m_vPlayers.find(idPlayer);

	if (v == m_vPlayers.end())
		return DPERR_INVALIDPLAYER;

	auto p = v->second;
	m_vPlayers.erase(v);

#ifdef _DEBUG
	printf("[LOADER] Player destroy %u\n", p->GetId());
#endif

	if (m_bHost && p->IsHostMade()) // Tell all the other players that a player disconnected
		enet_host_broadcast(m_pHost, ENET_CHANNEL_CHAT, DPMsg::DestroyPlayer(p));

	p->Disconnect();

	return DP_OK;
}

HRESULT DPInstance::CreatePlayer(LPDPID lpidPlayer, LPDPNAME lpPlayerName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	if (!m_bHost)
	{
		if (dwFlags & DPPLAYER_SERVERPLAYER)
			return DPERR_CANTCREATEPLAYER;

		if (!m_bConnected)
			return DPERR_NOCONNECTION;
	}

	if (m_bHost)
		*lpidPlayer = (DWORD)m_vPlayers.size() + 1;
	else
	{ // CLIENT: Ask the network for a new player id
		enet_peer_send(m_pClientPeer, ENET_CHANNEL_NORMAL, DPMsg::CallNewId(lpPlayerName));

#ifdef _DEBUG
		printf("[LOADER] Getting peer id from server...\n");
#endif

		while (true) // Idle until we receive the new id
		{
			Service(0);

			if (!m_bConnected)
			{
#ifdef _DEBUG
				printf("[LOADER] Connection lost\n");
#endif
				return DPERR_CONNECTIONLOST; // F
			}

			if (m_pClientPeer->data != 0)
			{
				*lpidPlayer = (DPID)m_pClientPeer->data;
				break;
			}
		}
	}

#ifdef _DEBUG
	printf("[LOADER] New player id %u\n", *lpidPlayer);
#endif

	const auto player = std::make_shared<DPPlayer>();
	player->Create(*lpidPlayer, lpPlayerName->lpszShortNameA, lpPlayerName->lpszLongNameA, hEvent, lpData, dwDataSize, dwFlags & DPPLAYER_SPECTATOR, dwFlags & DPPLAYER_SERVERPLAYER);

	if (m_bHost && player->IsHostMade())
	{
		// Tell all the other peers that a new player is online
		enet_host_broadcast(m_pHost, ENET_CHANNEL_CHAT, DPMsg::NewPlayer(player, (DWORD)m_vPlayers.size()));
	}


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
#ifdef _DEBUG
				printf("[LOADER] Disconnected! (timeout? %d)\n", evt.type == ENET_EVENT_TYPE_DISCONNECT_TIMEOUT);
#endif

				enet_peer_reset(m_pClientPeer);
				m_pClientPeer = nullptr;
				m_bConnected = false;
				m_vMessages.clear();

				DPMSG_SESSIONLOST msg2;
				msg2.dwType = DPSYS_SESSIONLOST;

				auto msg = std::make_shared<DPMsg>(0, 0, DPMSG_TYPE_SYSTEM);
				msg->AddToSerialize(msg2);

				// Remove all messages and push the session lost one, telling the app the we lost the connection				m_vMessages.clear();
				m_vMessages.push_back(msg);
			}
			else
			{ // SERVER
				if (evt.peer->data) // authenticated player
				{
					auto id = (DPID)evt.peer->data;

#ifdef _DEBUG
					printf("[LOADER] Peer %d disconnected! (timeout? %d)\n", id, evt.type == ENET_EVENT_TYPE_DISCONNECT_TIMEOUT);
#endif

					auto it = m_vPlayers.find(id);

					if (it == m_vPlayers.end())
						break; // how?

					auto p = it->second;

					// tell all the peers that a player disconnected

					auto r = std::make_shared<DPMsg>(DPMsg::DestroyPlayer(p), true);

					m_vMessages.push_back(r); // tell ourself that someone died

					enet_host_broadcast(m_pHost, ENET_CHANNEL_CHAT, DPMsg::DestroyPlayer(p));
				}

				break;
			}

			break;

		case ENET_EVENT_TYPE_CONNECT:
			if (!m_bHost)
			{
#ifdef _DEBUG
				printf("[LOADER] Client connected\n");
#endif
				m_bConnected = true;
			}
			else if (evt.data == 0)
			{
#ifdef _DEBUG
				printf("[LOADER] Sending game info to peer\n");
#endif

				// SERVER: Send game info to client
				enet_peer_send(evt.peer, ENET_CHANNEL_NORMAL, DPMsg::CreateRoomInfo(m_gSession, m_dwMaxPlayers, m_vPlayers.size(), m_szGameName.c_str(), m_adwUser, m_dwFlags));
				//enet_peer_disconnect(evt.peer, 0);
			}
			else
			{
#ifdef _DEBUG
				printf("[LOADER] New peer connect\n");
#endif
				break; // Do not add this internal message to the queue
			}

			enet_peer_timeout(evt.peer, TIMEOUT1, TIMEOUT2, TIMEOUT3);

			break;

		case ENET_EVENT_TYPE_RECEIVE:
		{
			auto msg = std::make_shared<DPMsg>(evt.packet, true);

#ifdef _DEBUG
			printf("[LOADER] Received %u\n", evt.packet->dataLength);
#endif

			if (m_bHost)
			{
				// Setup peer id and send it back
				if (msg->GetType() == DPMSG_TYPE_CALL_NEWID)
				{
					DPPlayerInfo* pInfo = (DPPlayerInfo*)msg->Read2(sizeof(DPPlayerInfo));
 
					DPID id = (DPID)m_vPlayers.size() + 1;
					enet_peer_send(evt.peer, ENET_CHANNEL_NORMAL, DPMsg::NewId(id));
					evt.peer->data = (LPVOID)id; // set id which means the player is authenticatedù

#ifdef _DEBUG
					printf("[LOADER] New peer id %u\n", id);
#endif

					for (const auto& pp : m_vPlayers)
     				{
						auto pinfo = pp.second;
						enet_peer_send(evt.peer, ENET_CHANNEL_NORMAL, DPMsg::NewPlayer(pinfo, (DWORD)m_vPlayers.size()));
					}

					auto pp = std::make_shared<DPPlayer>();
					pp->Create(id, pInfo->name[0] ? pInfo->name : nullptr, pInfo->longName[0] ? pInfo->longName : nullptr, nullptr, pInfo->dwDataSize ? msg->Read2(pInfo->dwDataSize) : nullptr, pInfo->dwDataSize, false, false);
					pp->SetPeer(evt.peer);
					            
					auto sMsg = std::make_shared<DPMsg>(DPMsg::NewPlayer(pp, m_vPlayers.size()), true);
					m_vMessages.push_back(sMsg);


					m_vPlayers.insert_or_assign(id, pp);

					break; // Do not add this internal message to the queue
				}
			}
			else
			{
				if (msg->GetType() == DPMSG_TYPE_NEWID)
				{
					evt.peer->data = (LPVOID)msg->Read2(sizeof(DPID)); // Assign the readed id
					m_pClientPeer->data = evt.peer->data;
#ifdef _DEBUG
					printf("[LOADER] Assigned peer id from server %u\n", (DPID)m_pClientPeer->data);
#endif
					break; // Do not add this internal message to the queue
				}
			}

			if (msg->GetType() == DPMSG_TYPE_REMOTEINFO)
			{
				auto p = m_vPlayers.find(msg->GetFrom());

				if (p != m_vPlayers.end())
				{
					DWORD len;
					msg->Read(len);
					p->second->SetRemoteData(msg->Read2(len), len);
					break; // Do not add this internal message to the queue
				}
			}

			if (evt.peer->data != 0)
			{
				auto p = m_vPlayers[(DPID)evt.peer->data];
				if (p.get())
				{
					if (msg->GetTo() == p->GetId())
						p->FireEvent(); // Fire handle event as specified by DirectPlay
				}
			}

			m_vMessages.push_back(msg);
			break;
		}
		}
	}
}

HRESULT DPInstance::SetPlayerData(DPID idPlayer, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	auto p = m_vPlayers.find(idPlayer);

	if (p == m_vPlayers.end())
		return DPERR_INVALIDPLAYER;

	if (dwFlags & DPSET_LOCAL)
		p->second->SetLocalData(lpData, dwDataSize);

	p->second->SetRemoteData(lpData, dwDataSize);

	// dwFlags & DPSET_GUARANTEED
	
	if (!m_bHost)
	{
		enet_peer_send(m_pClientPeer, ENET_CHANNEL_NORMAL, DPMsg::CreatePlayerRemote(p->second, dwFlags & DPSET_GUARANTEED));
	}
	else
	{
		enet_host_broadcast(m_pHost, ENET_CHANNEL_NORMAL, DPMsg::CreatePlayerRemote(p->second, dwFlags & DPSET_GUARANTEED));
	}

	return DP_OK;
}

HRESULT DPInstance::Open(LPDPSESSIONDESC2 lpsd, DWORD dwFlags)
{
	if (lpsd->dwFlags & DPCAPS_ASYNCSUPPORTED)
		return DPERR_UNAVAILABLE; // No

	Globals::Get()->TheArena->Reset(); // attempt to flush cache by resetting the arena

	if (dwFlags == DPOPEN_CREATE)
	{
		if (m_pHost)
			return DPERR_ALREADYINITIALIZED;

#ifdef _DEBUG
		printf("[LOADER] Creating new match...\n");
#endif

		m_guidFF = lpsd->guidApplication;

		ENetAddress addr;
		addr.port = (uint16_t)FURFIGHTERS_PORT;

		enet_address_set_ip(&addr, "0.0.0.0");

		if (FAILED(CoCreateGuid(&m_gSession)))
			return DPERR_CANNOTCREATESERVER;

		m_pHost = enet_host_create(&addr, lpsd->dwMaxPlayers, ENET_CHANNEL_MAX, 0, 0, ENET_BUFFER_SIZE);

		if (!m_pHost)
			return DPERR_CANNOTCREATESERVER;

#ifdef _DEBUG
		printf("[LOADER] Creation ok: max players %u\n", lpsd->dwMaxPlayers);
#endif

		m_vMessages.clear();
		m_bHost = true;
		m_szGameName = lpsd->lpszSessionNameA;
		m_dwMaxPlayers = lpsd->dwMaxPlayers;
		m_dwFlags = lpsd->dwFlags;
	}
	else if (dwFlags == DPOPEN_JOIN)
	{
		m_bHost = false;
		m_guidFF = lpsd->guidApplication;

		if (m_pClientPeer)
		{
			enet_peer_disconnect(m_pClientPeer, 0);
			Service(1000);
			m_pClientPeer = nullptr;
		}

		char addr[40] = { 0 };
		enet_address_get_ip(&m_eConnectAddr, addr, 40);

#ifdef _DEBUG
		printf("[LOADER] Trying to connect to %s:%u...\n", addr, m_eConnectAddr.port);
#endif

#if 0
		auto it = m_vEnumAddr.find(lpsd->guidInstance);

		if (it == m_vEnumAddr.end())
			return DPERR_INVALIDPARAMS;

		m_pClientPeer = enet_host_connect(m_pHost, &it->second, ENET_CHANNEL_MAX, 1);
#else
		m_pClientPeer = enet_host_connect(m_pHost, &m_eConnectAddr, ENET_CHANNEL_MAX, 1);
#endif

		if (!m_pClientPeer)
			return DPERR_NOCONNECTION;

		enet_peer_timeout(m_pClientPeer, TIMEOUT1, TIMEOUT2, TIMEOUT3);

		m_vMessages.clear();

#ifdef _DEBUG
		printf("[LOADER] Connect creation ok, start service...\n");
#endif

		//return DPERR_CONNECTING;

		Service(ENET_SERVICE_TIME);

		if (!m_bConnected)
		{
#ifdef _DEBUG
			printf("[LOADER] Connection failed\n");
#endif
			enet_peer_reset(m_pClientPeer);
			m_pClientPeer = nullptr;
			return DPERR_NOCONNECTION;
		}

#ifdef _DEBUG
		printf("[LOADER] Connection ok\n");
#endif
	}

	return DP_OK;
}

HRESULT DPInstance::Close(void)
{
	m_bService = false;

	xlog(DPINSTANCE, DEBUG2, "Shutdown connection");

	if (m_thread.joinable())
		m_thread.join();

	if (m_pHost)
	{
		xlog(DPINSTANCE, DEBUG, "Service disconnect");

		Service(ENET_SERVICE_TIME); // let everything disconnect gracefully

		if (m_pClientPeer) // CLIENT: reset peer
		{
			enet_peer_reset(m_pClientPeer);
			m_pClientPeer = nullptr;
		}

		enet_host_destroy(m_pHost);
		m_pHost = nullptr;
	}

	m_bConnected = false;

	return DP_OK;
}

HRESULT DPInstance::InitializeConnection(LPVOID lpConnection, DWORD dwFlags)
{
	if (!lpConnection)
		return DPERR_INVALIDPARAMS; // We do not support discovery

	LPDPADDRESS addr = (LPDPADDRESS)lpConnection;

	if (lpConnection)
	{ // ASYNC
#ifdef _DEBUG
		printf("[LOADER] Setup address for client mode...\n");
#endif

		if (m_pHost)
			return DPERR_ALREADYINITIALIZED;

		// Client needs host created immidiatly so we can connect and query game info
		m_pHost = enet_host_create(nullptr, 1, ENET_CHANNEL_MAX, 0, 0, ENET_BUFFER_SIZE);

		if (!m_pHost)
			return DPERR_UNINITIALIZED;

		ENetAddress eAddr;
		if (!GetAddressFromDPAddress(addr, &eAddr))
			return DPERR_UNINITIALIZED;

		char addr4[40];
		enet_address_get_ip(&eAddr, addr4, 40);

#ifdef _DEBUG
		printf("[LOADER] Setup address %s:%d\n", addr4, eAddr.port);
#endif

		m_eConnectAddr = eAddr;
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
				out->port = (uint16_t)FURFIGHTERS_PORT;
				setIp = true;
				break; // possible fix for addr2 point derefence
			}

			/*else if (InlineIsEqualGUID(addr2->guidDataType, DPAID_INetPort))
			{
				out->port = *(USHORT*)(b + i + sizeof(DPADDRESS));
			}*/

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

	xlog(DPINSTANCE, DEBUG, "Enum connection %s\n", dpName.lpszLongNameA);

	lpEnumCallback(&GUID_ENet, nullptr, 0, &dpName, 0, lpContext);

	return DP_OK;
}

HRESULT DPInstance::GetSessionDesc(LPVOID lpData, LPDWORD lpdwDataSize)
{
	if (*lpdwDataSize < sizeof(DPSESSIONDESC2))
		return DPERR_BUFFERTOOSMALL;

	if (lpData)
	{
		LPDPSESSIONDESC2 desc = (LPDPSESSIONDESC2)lpData;
		desc->dwSize = sizeof(DPSESSIONDESC2);
		desc->dwFlags = 0;
		desc->guidApplication = m_guidFF;
		desc->guidInstance = m_gSession;
		desc->dwMaxPlayers = m_dwMaxPlayers;
		desc->dwCurrentPlayers = (DWORD)m_vPlayers.size();
		desc->dwReserved1 = 0;
		desc->dwReserved2 = 0;
		desc->dwUser1 = m_adwUser[0];
		desc->dwUser2 = m_adwUser[1];
		desc->dwUser3 = m_adwUser[2];
		desc->dwUser4 = m_adwUser[3];
		desc->lpszPasswordA = nullptr;
		desc->lpszSessionNameA = (LPSTR)m_szGameName.c_str();
		desc->dwFlags = m_dwFlags;
	}

	return DP_OK;
}

HRESULT DPInstance::GetPlayerData(DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags)
{
	auto p = m_vPlayers.find(idPlayer);

	if (m_vPlayers.end() == p)
		return DPERR_INVALIDPLAYER;

	DWORD r;
	LPVOID v;

	if (dwFlags & DPGET_LOCAL)
	{
		r = p->second->GetLocalDataSize();
		v = p->second->GetLocalData();
	}
	else
	{
		r = p->second->GetRemoteDataSize();
		v = p->second->GetRemoteData();
	}

	if (r > *lpdwDataSize)
		return DPERR_BUFFERTOOSMALL;

	memcpy_s(lpData, *lpdwDataSize, v, r);
	*lpdwDataSize = r;
	return DP_OK;
}

void DPInstance::SetupThreadedService(bool infinite)
{
#ifdef _DEBUG
	printf("[LOADER] Start enet service thread...\n");
#endif

	m_bService = true;
	m_ullStartService = GetTickCount64();

	if (infinite)
		m_ullServiceTimeout = UINT64_MAX;

	m_thread = std::thread([this]() {
		while (m_bService)
		{
			if ((GetTickCount64() - m_ullStartService) > m_ullServiceTimeout)
				Service(0);
		}
	});
}

HRESULT DPInstance::EnumSessionOut(LPDPENUMSESSIONSCALLBACK2 cb, LPVOID ctx)
{
	m_bService = false;

	if (m_thread.joinable())
		m_thread.join();

#ifdef _DEBUG
	printf("[LOADER] Start session output... (%zu)\n", m_vMessages.size());
#endif

	for (auto it2 = m_vMessages.begin(); it2 != m_vMessages.end(); it2++)
	{
		auto it = (*it2);

#ifdef _DEBUG
		printf("[LOADER] EnumSession msg %d %d %d\n", it->GetType(), it->GetFrom(), it->GetTo());
#endif

		if (it->GetType() == DPMSG_TYPE_GAME_INFO)
		{
			DWORD r = 0;
			it->FixSysMessage(nullptr, &r);

			DPGameInfo* info = (DPGameInfo*)it->Read2(sizeof(DPGameInfo));
			DPSESSIONDESC2 desc;
			desc.dwSize = sizeof(desc);
			desc.dwFlags = info->flags;
			desc.dwUser1 = info->user[0];
			desc.dwUser2 = info->user[1];
			desc.dwUser3 = info->user[2];
			desc.dwUser4 = info->user[3];
			desc.dwReserved1 = 0;
			desc.dwReserved2 = 0;
			desc.lpszSessionNameA = info->sessionName;
			desc.lpszPasswordA = nullptr;
			desc.guidApplication = m_guidFF;
			desc.guidInstance = info->session;
			desc.dwMaxPlayers = info->maxPlayers;
			desc.dwCurrentPlayers = info->currPlayers;

			m_dwMaxPlayers = info->maxPlayers;
			m_gSession = info->session;
			m_szGameName = info->sessionName;
			m_dwFlags = info->flags;

#ifdef _DEBUG
			printf("[LOADER] EnumSession got lobby %s\n", info->sessionName);
#endif

			DWORD stub = 0;
			cb(&desc, &stub, 0, ctx);

			m_vMessages.clear();
			return DP_OK;
		}
	}

#ifdef _DEBUG
	printf("[LOADER] EnumSession: no lobbies found\n");
#endif
	return DPERR_NOCONNECTION;
}

DPMsg DPInstance::MakeMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize)
{
	DPMsg msg(idFrom, idTo, DPMSG_TYPE_GAME);
	msg.AddToSerialize(dwDataSize);

	if (lpData)
	{
		msg.AddToSerialize(lpData, dwDataSize);

		std::string szDbgMsg = "Message: ";

		for (DWORD m = 0; m < dwDataSize; m++)
		{
			char buf[16];
			_snprintf_s(buf, _countof(buf), "%x ", (int)*((LPBYTE*)(lpData)+m));
			szDbgMsg += buf;
		}

		xlog(DPINSTANCE, NET, szDbgMsg.c_str());
	}

	return msg;
}
