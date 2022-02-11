/*!
	@author Arves100
	@brief DirectPlay message reimplementation
	@date 08/02/2022
	@file DPMsg.cpp
*/
#include "stdafx.h"
#include "DPMsg.h"
#include "Globals.h"

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

ENetPacket* DPMsg::CallNewId(LPDPNAME lpData)
{
	DPMsg msg(0, 0, DPMSG_TYPE_CALL_NEWID);
	DPPlayerInfo nfo = { 0 };

	if (lpData->lpszLongNameA)
		strncpy_s(nfo.longName, 100, lpData->lpszLongNameA, 100);
	if (lpData->lpszShortNameA)
		strncpy_s(nfo.name, 40, lpData->lpszShortNameA, 40);
	
	msg.AddToSerialize(nfo);

	return msg.Serialize();
}

ENetPacket* DPMsg::NewId(DPID id)
{
	DPMsg msg(DPID_SYSMSG, DPID_SYSMSG, DPMSG_TYPE_NEWID);
	msg.AddToSerialize(id);
	return msg.Serialize();
}

ENetPacket* DPMsg::CreateRoomInfo(GUID roomId, DWORD maxPlayers, DWORD currPlayers, const char* sessionName, DWORD user[4], DWORD dwFlags)
{
	DPGameInfo info;
	info.session = roomId;
	info.maxPlayers = maxPlayers;
	info.currPlayers = currPlayers;
	strncpy_s(info.sessionName, _countof(info.sessionName), sessionName, 100);
	memcpy_s(info.user, sizeof(info.user), user, sizeof(info.user));
	info.flags = dwFlags;
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
	size_t len = strlen(chatMsg->lpszMessageA);
	msg.AddToSerialize(len);
	msg.AddToSerialize(chatMsg->lpszMessageA, len + 1);
	return msg.Serialize(reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
}

ENetPacket* DPMsg::CreatePlayerRemote(const std::shared_ptr<DPPlayer>& player, bool reliable)
{
	DPMsg msg(player->GetId(), 0, DPMSG_TYPE_REMOTEINFO);

	DWORD m = player->GetRemoteDataSize();
	msg.AddToSerialize(m);

	if (m > 0)
		msg.AddToSerialize(player->GetRemoteData(), m);

	return msg.Serialize(reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
}

ENetPacket* DPMsg::CreateSendComplete(DPID idFrom, DPID idTo, DWORD dwFlags, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, DWORD lpdwMsgID, HRESULT hr, DWORD dwSendTime)
{
	DPMsg msg(idFrom, idTo, DPMSG_TYPE_SYSTEM);
	DPMSG_SENDCOMPLETE msg2;
	msg2.dwType = DPSYS_SENDCOMPLETE;
	msg2.dwTimeout = dwTimeout;
	msg2.idFrom = idFrom;
	msg2.idTo = idTo;
	msg2.dwFlags = dwFlags;
	msg2.dwPriority = dwPriority;
	msg2.dwMsgID = lpdwMsgID;
	msg2.lpvContext = lpContext;
	msg2.hr = hr;
	msg2.dwSendTime = dwSendTime;
	msg.AddToSerialize(msg2);
	return msg.Serialize();
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

	if (!p)
		return DPERR_GENERIC;

	auto arena = Globals::Get()->TheArena;

	switch (p->dwType)
	{
	case DPSYS_SENDCOMPLETE:
	{
		DPMSG_SENDCOMPLETE* msg = (DPMSG_SENDCOMPLETE*)Read2(sizeof(DPMSG_SENDCOMPLETE));
		reqSize = sizeof(DPMSG_SENDCOMPLETE);
		break;
	}
	case DPSYS_CREATEPLAYERORGROUP:
	{
		DPMSG_CREATEPLAYERORGROUP* msg = (DPMSG_CREATEPLAYERORGROUP*)Read2(sizeof(DPMSG_CREATEPLAYERORGROUP));
		DPNameNet* nm = (DPNameNet*)arena->Store(Read2(sizeof(DPNameNet)), sizeof(DPNameNet));

		if (msg->dwDataSize)
			msg->lpData = arena->Store(Read2(msg->dwDataSize), msg->dwDataSize);

		msg->dpnName.lpszLongNameA = nm->longName;
		msg->dpnName.lpszShortNameA = nm->shortName;

		reqSize = sizeof(DPMSG_CREATEPLAYERORGROUP);
		break;
	}
	case DPSYS_DESTROYPLAYERORGROUP:
	{
		DPMSG_DESTROYPLAYERORGROUP* msg = (DPMSG_DESTROYPLAYERORGROUP*)Read2(sizeof(DPMSG_DESTROYPLAYERORGROUP));
		reqSize = sizeof(DPMSG_DESTROYPLAYERORGROUP);
		break;
	}
	case DPSYS_CHAT:
	{
		DPMSG_CHAT* msg = (DPMSG_CHAT*)Read2(sizeof(DPMSG_CHAT));
		LPDPCHAT lpc = (LPDPCHAT)arena->Store(Read2(sizeof(DPCHAT)), sizeof(DPCHAT));

		msg->lpChat = lpc;

		size_t len;
		Read(len);
		len += 1;

		lpc->lpszMessageA = (LPSTR)arena->Store(Read2(len), len);

		reqSize = sizeof(DPMSG_CHAT);
		break;
	}
	default:
		return DPERR_INVALIDOBJECT;
	}

	if (lpData && *lpDataSize < reqSize)
		return DPERR_BUFFERTOOSMALL;

	if (lpData)
		memcpy_s(lpData, *lpDataSize, p, reqSize);

	*lpDataSize = reqSize;
	return DP_OK;
}
