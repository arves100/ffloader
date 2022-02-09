/*!
	@author Arves100
	@brief DirectPlay message reimplementation
	@date 08/02/2022
	@file DPMsg.cpp
*/
#include "stdafx.h"
#include "DPMsg.h"

// Destroy message:
//		DPMSG_DESTROYPLAYERORGROUP
//		[Local data]
//		[Remote data]

// Create message:
//		DPMSG_CREATEPLAYERORGROUP
//		DPNameNet
//		[Local data]

struct DPNameNet
{
	char shortName[30];
	char longName[100];
};

ENetPacket* DPMsg::DestroyPlayer(const std::shared_ptr<DPPlayer>& player)
{
	DPMSG_DESTROYPLAYERORGROUP msg;

	msg.dwType = DPSYS_DESTROYPLAYERORGROUP;
	msg.dwPlayerType = DPPLAYERTYPE_PLAYER;
	msg.dpId = player->GetId();
	msg.lpLocalData = nullptr; // See above
	msg.dwLocalDataSize = player->GetLocalDataSize();
	msg.dwRemoteDataSize = player->GetRemoteDataSize();
	msg.lpRemoteData = nullptr; // See avobe
	msg.dpIdParent = 0;
	msg.dwFlags = player->IsSpecator() ? DPPLAYER_SPECTATOR : 0;
	msg.dwFlags |= player->IsMadeByHost() ? DPPLAYER_SERVERPLAYER : 0;

	DPMsg dpMsg(player->GetId(), DPID_SYSMSG, DPMSG_TYPE_SYSTEM);
	dpMsg.AddToSerialize(msg);

	if (msg.dwLocalDataSize)
		dpMsg.AddToSerialize(player->GetLocalData(), msg.dwLocalDataSize);

	if (msg.dwRemoteDataSize)
		dpMsg.AddToSerialize(player->GetRemoteData(), msg.dwRemoteDataSize);

	return dpMsg.Serialize();
}

ENetPacket* DPMsg::NewPlayer(const std::shared_ptr<DPPlayer>& player, DWORD oldPlayer)
{
	DPMSG_CREATEPLAYERORGROUP msg;
	msg.dwType = DPSYS_CREATEPLAYERORGROUP;
	msg.dwPlayerType = DPPLAYERTYPE_PLAYER;
	msg.dpId = player->GetId();
	msg.dwCurrentPlayers = oldPlayer;
	msg.lpData = nullptr;
	msg.dwDataSize = player->GetLocalDataSize();

	DPNAME name;
	name.dwSize = sizeof(name);
	name.dwFlags = 0;
	name.lpszLongNameA = nullptr;
	name.lpszShortNameA = nullptr;
	msg.dpnName = name;

	msg.dpIdParent = 0;
	msg.dwFlags = 0;

	DPMsg dpMsg(DPID_SYSMSG, DPID_SYSMSG, DPMSG_TYPE_SYSTEM);
	dpMsg.AddToSerialize(msg);

	DPNameNet netName;
	strcpy_s(netName.shortName, _countof(netName.shortName), player->GetShortName());
	strcpy_s(netName.longName, _countof(netName.longName), player->GetLongName());
	dpMsg.AddToSerialize(netName);

	if (msg.dwDataSize)
		dpMsg.AddToSerialize(player->GetLocalData(), msg.dwDataSize);

	return dpMsg.Serialize();
}

ENetPacket* DPMsg::CallNewId()
{
	return DPMsg(0, 0, DPMSG_TYPE_CALL_NEWID).Serialize();
}

ENetPacket* DPMsg::NewId(DPID id)
{
	DPMsg msg(DPID_SYSMSG, DPID_SYSMSG, DPMSG_TYPE_NEWID);
	msg.AddToSerialize(id);
	return msg.Serialize();
}

ENetPacket* DPMsg::CreateRoomInfo(GUID roomId, DWORD maxPlayers, DWORD currPlayers, const char* sessionName, DWORD user[4])
{
	DPGameInfo info;
	info.session = roomId;
	info.maxPlayers = maxPlayers;
	info.currPlayers = currPlayers;
	strncpy_s(info.sessionName, _countof(info.sessionName), sessionName, 100);
	memcpy_s(info.user, sizeof(info.user), user, sizeof(info.user));
	DPMsg msg(DPID_SYSMSG, DPID_SYSMSG, DPMSG_TYPE_GAME_INFO);
	msg.AddToSerialize(info);
	return msg.Serialize();
}

ENetPacket* DPMsg::ChatPacket(DPID from, DPID to, bool reliable, LPDPCHAT chatMsg)
{
	DPMSG_CHAT chat;
	chat.dwType = DPSYS_CHAT;
	chat.dwFlags = 0;
	chat.idFromPlayer = from;
	chat.idToGroup = 0;
	chat.idToPlayer = to;
	chat.lpChat = chatMsg;

	DPMsg msg(from, to, DPMSG_TYPE_SYSTEM);
	msg.AddToSerialize(chat);
	msg.AddToSerialize(chatMsg, sizeof(DPCHAT));
	msg.AddToSerialize(chatMsg->lpszMessageA, chatMsg->dwSize);
	return msg.Serialize(reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
}

/*!
* @brief Translates internal network messages to DirectPlay messages
* @return HResult error code or DP_OK in case of success
* @param lpData Pointer of the data to store
* @param lpDataSize Pointer of the size of the data to store
*/
HRESULT_INT DPMsg::FixSysMessage(LPVOID lpData, LPDWORD lpDataSize)
{
	ResetRead();

	auto p = (DPMSG_GENERIC*)Read2(sizeof(DPMSG_GENERIC));
	DWORD reqSize = 0;
	ResetRead();

	switch (p->dwType)
	{
	case DPSYS_CREATEPLAYERORGROUP:
	{
		DPMSG_CREATEPLAYERORGROUP* msg = (DPMSG_CREATEPLAYERORGROUP*)Read2(sizeof(DPMSG_CREATEPLAYERORGROUP));
		DPNameNet* nm = (DPNameNet*)Read2(sizeof(DPNameNet));

		if (msg->dwDataSize)
			msg->lpData = (LPVOID)Read2(msg->dwDataSize);

		msg->dpnName.lpszLongNameA = nm->longName;
		msg->dpnName.lpszShortNameA = nm->shortName;
		reqSize = sizeof(DPMSG_CREATEPLAYERORGROUP);
		break;
	}
	case DPSYS_DESTROYPLAYERORGROUP:
	{
		DPMSG_DESTROYPLAYERORGROUP* msg = (DPMSG_DESTROYPLAYERORGROUP*)Read2(sizeof(DPMSG_DESTROYPLAYERORGROUP));

		if (msg->dwLocalDataSize)
			msg->lpLocalData = Read2(msg->dwLocalDataSize);

		if (msg->dwRemoteDataSize)
			msg->lpRemoteData = Read2(msg->dwRemoteDataSize);

		reqSize = sizeof(DPMSG_DESTROYPLAYERORGROUP);
		break;
	}
	case DPSYS_CHAT:
	{
		DPMSG_CHAT* msg = (DPMSG_CHAT*)Read2(sizeof(DPMSG_CHAT));
		LPDPCHAT lpc = (LPDPCHAT)Read2(sizeof(DPCHAT));
		msg->lpChat = lpc;
		lpc->lpszMessageA = (char*)Read2(lpc->dwSize);
		reqSize = sizeof(DPMSG_CHAT);
		break;
	}
	default:
		return DPERR_INVALIDOBJECT;
	}

	if (lpData && *lpDataSize < reqSize)
		return DPERR_BUFFERTOOSMALL;

	if (lpData)
	{
		memcpy_s(lpData, *lpDataSize, p, reqSize);
		m_pPk = nullptr; // TODO: THIS IS EXTREMELY BAD FIND A WAY TO AVOID THIS AT ANY COST
	}

	*lpDataSize = reqSize;
	return DP_OK;
}
