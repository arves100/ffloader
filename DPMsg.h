/*!
	@author Arves100
	@brief DirectPlay message reimplementation
	@date 08/02/2022
	@file DPMsg.h
*/
#pragma once

#include "DPPlayer.h"

enum DPMsgTypes
{
	DPMSG_TYPE_SYSTEM = 0,
	DPMSG_TYPE_NEWID = 1,
	DPMSG_TYPE_CALL_NEWID = 2,
	DPMSG_TYPE_GAME_INFO = 3,
};

struct DPGameInfo
{
	GUID session;
	DWORD maxPlayers;
	DWORD currPlayers;
	char sessionName[100];
	DWORD user[4];
};

/*!
	@class DPMsg
	Serializer/Deserializer of NetLib network messages
*/
class DPMsg
{
public:
	DPMsg(DPID from, DPID to, BYTE type)
	{
		m_header.from = from;
		m_header.to = to;
		m_header.type = type;
		m_lpRaw = nullptr;
		m_nRawTotalSize = 0;
		m_pPk = nullptr;
		m_nOffset = 0;
	}

	// Deserialize
	DPMsg(ENetPacket* ref, bool hold)
	{
		m_pPk = nullptr;
		m_nRawTotalSize = 0;
		m_nOffset = 0;

		Deserialize(ref, hold);
	}

	~DPMsg()
	{
		if (m_pPk)
		{
			enet_packet_destroy(m_pPk);
		}
	}

	void Deserialize(ENetPacket* ref, bool hold)
	{
		m_header = *(Header*)ref->data;
		m_lpRaw = ref->data + sizeof(Header);

		if (hold)
			m_pPk = ref;

		m_nRawTotalSize = ref->dataLength - sizeof(Header);
		m_nOffset = sizeof(Header);
	}

	template <typename T>
	void AddToSerialize(T data)
	{
		m_vRawData.push_back({ &data, sizeof(data)});
		m_nRawTotalSize += sizeof(data);
	}

	void AddToSerialize(LPVOID data, size_t len)
	{
		m_vRawData.push_back({ data, len });
		m_nRawTotalSize += len;
	}

	ENetPacket* Serialize(uint32_t flag = ENET_PACKET_FLAG_RELIABLE)
	{
		auto pk = enet_packet_create(nullptr, sizeof(Header) + m_nRawTotalSize, flag);
		size_t cnt = sizeof(m_header);

		memcpy_s(pk->data, pk->dataLength, &m_header, sizeof(m_header));

		for (const auto& p : m_vRawData)
		{
			memcpy_s(pk->data + cnt, pk->dataLength - cnt, p.data, p.len);
			cnt += p.len;
		}

		return pk;
	}

	void ResetRead()
	{
		m_nOffset = sizeof(Header);
	}

	LPBYTE Read2(size_t len)
	{
		if ((m_nOffset + len) > m_nRawTotalSize)
			return nullptr;

		auto p = m_lpRaw + m_nOffset;
		m_nOffset += len;
		return p;
	}

	template <typename T>
	void Read(T& m)
	{
		m = *(T*)Read2(sizeof(m));
	}

	DPID GetFrom() const { return m_header.from; }
	DPID GetTo() const { return m_header.to; }
	BYTE GetType() const { return m_header.type; }

	/*!
	* @brief Translates internal network messages to DirectPlay messages
	* @return HResult error code or DP_OK in case of success
	* @param lpData Pointer of the data to store
	* @param lpDataSize Pointer of the size of the data to store
	*/
	HRESULT_INT FixSysMessage(LPVOID lpData, LPDWORD lpDataSize);

	static ENetPacket* NewPlayer(const std::shared_ptr<DPPlayer>& player, DWORD oldPlayer);
	static ENetPacket* DestroyPlayer(const std::shared_ptr<DPPlayer>& player);
	static ENetPacket* CallNewId();
	static ENetPacket* NewId(DPID id);
	static ENetPacket* CreateRoomInfo(GUID roomId, DWORD maxPlayers, DWORD currPlayers, const char* sessionName, DWORD user[4]);

private:
	struct RefData
	{
		LPVOID data;
		size_t len;
	};

	struct Header
	{
		DPID from;
		DPID to;
		BYTE type;
	} m_header;

	// Used for serliazation

	std::vector<RefData> m_vRawData;
	size_t m_nRawTotalSize;

	// Used for deserialization get
	size_t m_nOffset;
	LPBYTE m_lpRaw;

	// Used for memory cleanup

	ENetPacket* m_pPk;
};
