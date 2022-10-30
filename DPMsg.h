/*!
	@author Arves100
	@brief DirectPlay message reimplementation
	@date 08/02/2022
	@file DPMsg.h
*/
#pragma once

#include "DPPlayer.h"

/*!
	@enum DPMsgTypes
	ENet wrapper DirectPlay message types
*/
enum DPMsgTypes
{
	/// System messages
	DPMSG_TYPE_SYSTEM = 0,

	/// New user id
	DPMSG_TYPE_NEWID = 1,

	/// Request for a new user id
	DPMSG_TYPE_CALL_NEWID = 2,

	/// Lobby info
	DPMSG_TYPE_GAME_INFO = 3,

	/// Chat message
	DPMSG_TYPE_CHAT = 4,

	/// Game message
	DPMSG_TYPE_GAME = 5,

	/// Information on a remote peer
	DPMSG_TYPE_REMOTEINFO = 6,
};

struct DPGameInfo
{
	GUID session;
	DWORD maxPlayers;
	DWORD currPlayers;
	char sessionName[100];
	DWORD user[4];
	DWORD flags;
};

struct DPPlayerInfo
{
	DPID id;
	char name[40];
	char longName[100];
	DWORD curr;
	DWORD dwDataSize;
};

/*!
	@class DPMsg
	Serializer/Deserializer of NetLib network messages
*/
class DPMsg
{
public:
	/*!
		Constructor for serializer
		@param from ID where the packet was received
		@param to ID where the packet is sent
		@param type Type of packet
	*/
	DPMsg(DPID from, DPID to, BYTE type)
	{
		// header setup
		m_header.from = from;
		m_header.to = to;
		m_header.type = type;

		m_pPk = nullptr;

		m_nOffset = 0;
		
		xlog(DPMSG, DEBUG, "Create from=%d to=%d type=%d", from, to, type);
	}

	/*!
		Constructor for deserializer
		@param ref Enet packet to read
		@param hold Whenever to hold the packet or not
	*/
	DPMsg(ENetPacket* ref, bool hold)
	{
		m_pPk = nullptr;

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
		xlog(DPMSG, DEBUG, "Deserialize header %d %d %d", m_header.from, m_header.to, m_header.type);

		// setup raw data
		m_lpRaw = ref->data + sizeof(Header);

		if (hold)
			m_pPk = ref;

		m_nRawTotalSize = ref->dataLength - sizeof(Header);
		m_nOffset = 0;
	}

	template <typename T>
	void AddToSerialize(T& data)
	{
		AddToSerialize(&data, sizeof(data));
	}

	void AddToSerialize(LPVOID data, size_t len)
	{
		xlog(DPMSG, DEBUG2, "push data %d (total=%zu, offset=%zu)", sizeof(data), m_nRawTotalSize, m_nOffset);

		m_nRawTotalSize += len;

		m_vRaw.resize(m_nRawTotalSize);
		memcpy_s(&m_vRaw[m_nOffset], m_vRaw.size() - m_nOffset, data, len);

		m_nOffset += len;
	}

	ENetPacket* Serialize(uint32_t flag)
	{
		auto pk = enet_packet_create(nullptr, sizeof(Header) + m_nRawTotalSize, flag);

		memcpy_s(pk->data, pk->dataLength, &m_header, sizeof(m_header));
		memcpy_s(pk->data + sizeof(Header), pk->dataLength - sizeof(Header), m_vRaw.data(), m_nRawTotalSize);

		xlog(DPMSG, DEBUG2, "making up packet (header %d %d %d) size=%d, flag=%d", m_header.from, m_header.to, m_header.type, m_nRawTotalSize, flag);
		
		m_nOffset = 0;
		m_nRawTotalSize = 0;
		m_vRaw.clear();

		return pk;
	}

	void ResetRead()
	{
		m_nOffset = 0;
	}

	LPBYTE Read2(size_t len)
	{
		xlog(DPMSG, DEBUG2, "reading data=%zu offset=%zu total=%zu", len, m_nOffset, m_nRawTotalSize);

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
	size_t GetRawSize() const { return m_nRawTotalSize; }
	LPBYTE GetRaw() const { return m_lpRaw; }

	/*!
	* @brief Translates internal network messages to DirectPlay messages
	* @return HResult error code or DP_OK in case of success
	* @param lpData Pointer of the data to store
	* @param lpDataSize Pointer of the size of the data to store
	*/
	HRESULT_INT FixSysMessage(LPVOID lpData, LPDWORD lpDataSize);

	static ENetPacket* NewPlayer(const std::shared_ptr<DPPlayer>& player, DWORD oldPlayer);
	static ENetPacket* DestroyPlayer(const std::shared_ptr<DPPlayer>& player);
	static ENetPacket* CallNewId(LPDPNAME lpData);
	static ENetPacket* NewId(DPID id);
	static ENetPacket* CreateRoomInfo(GUID roomId, DWORD maxPlayers, DWORD currPlayers, const char* sessionName, DWORD user[4], DWORD dwFlags);

	/*!
		Makes a new chat packet
		@param from Peer from
		@param to Peer to
		@param data Chat data
		@param reliable If the message has to be sent in a reliable way
		@return Newly created packet
	*/
	static ENetPacket* ChatPacket(DPID from, DPID to, LPDPCHAT data, bool reliable);

	static ENetPacket* CreatePlayerRemote(const std::shared_ptr<DPPlayer>& player, bool reliable);
	static ENetPacket* CreateSendComplete(DPID idFrom, DPID idTo, DWORD dwFlags, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, DWORD lpdwMsgID, HRESULT hr, DWORD dwSendTime);

private:
	/*!
		@struct Header
		DirectPlay message header
	*/
	struct Header
	{
		/// Player who sent it
		DPID from;

		/// Player who received it
		DPID to;

		/// Type of message sent
		BYTE type;
	} m_header;

	// Used for deserialization get
	size_t m_nOffset;

	/// Pointer to raw data
	LPBYTE m_lpRaw;

	/// Raw size of the data
	size_t m_nRawTotalSize;

	/// Holder of enet packet
	ENetPacket* m_pPk;

	/// Buffer for raw data
	std::vector<uint8_t> m_vRaw;
};
