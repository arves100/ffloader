/*!
	@author Arves100
	@brief DirectPlay message reimplementation
	@date 08/02/2022
	@file Msg.h
*/
#pragma once

/// Maximum buffer of messages to write
#define MAX_MSG_BUFFER 256

/// Message header (version 0)
#define MSG_HEADER 0xDF00

/*!
    ENet Channels
*/
enum class DPChannel : uint8_t
{
    /// Normal channel (system or game)
    Normal,

    /// Chat channel
    Chat,

    /// Maximum channels
    Max,
};

/*!
    Types of messages
*/
enum class DPMsgType : uint8_t
{
    /// System message
    System,

    /// Game specific message
    Game,

    /// A chat message
    Chat,
};

/*!
    Types of system messages
*/
enum class DPMsgSystemType : uint8_t
{
    /// A player has been connected
    CreatePlayer,

    /// A player has been disconnected
    DestroyPlayer,

    /// A player has been timed out
    TimeoutPlayer,

    /// A peer has requested the authentication
    AuthenticationReq,

    /// The server answers the authentication
    AuthenticationRes,

    /// A peer asks for the game room information
    GameInfoReq,

    /// The server answers the game information
    GameInfoRes,
};

/*!
    Authentication response types
*/
enum class DPMsgAuthenticationRes : uint8_t
{
    /// Authentication ok
    Success,

    /// Lobby is currently full
    LobbyFull,

    /// The password is invalid
    InvalidPassword,

    /// Unknown error
    UnknownError,
};

/*!
    Header of a DirectPlay custom message
*/
struct DPMsgHeader
{
    /// Magic of the header
    uint32_t magic;

    /// Who sent the message
    DPID from;

    /// Who should receive the message
    DPID to;

    /// Type of the message (@see DPMsgType)
    uint8_t type;

    /// Size of the message without the header size
    size_t size;
};

/*!
    Information of the system message
*/
struct DPMsgSystem
{
    /// ID of the system message
    uint8_t systemID;

    /// An extra ID of a peer (which may be usefull in various things)
    DPID extraPeerID;

    /// An extra information of the packet
    uint32_t extraData;

    /*!
        Default constructor
    */
    explicit DPMsgSystem() : systemID(0), extraPeerID(DPID_INVALID), extraData(0) {}
};

/*!
    Internal serializer/deserialzier of a message
*/
struct DPMsg
{
    /// true if the message is invalid
    bool invalid;

    /// header of the message
    DPMsgHeader header;

    /// channel which the message should be sent
    uint8_t channel;

    /// packet flags (eg: reliable)
    uint8_t flags;

    /// custom data of the message (may be null)
    uint8_t data[MAX_MSG_BUFFER];

    /*!
        Default constructor
    */
    DPMsg() : invalid(true), channel(0), flags(0)
    {
        memset(&header, 0, sizeof(header));
        memset(data, 0, sizeof(data));
    }

    /*!
        Deserializes a message from a ENet packet
        @param pk Packet to deserialize
    */
    bool Read(ENetPacket* pk)
    {
        if (pk->dataLength < sizeof(uint32_t))
            return false;

        memcpy(&header.magic, pk->data, sizeof(header.magic));

        if (header.magic != MSG_HEADER)
            return false;

        memcpy(&header, pk->data, sizeof(header));

        if (header.size > MAX_MSG_BUFFER)
            return false; // limited for now...

        switch (header.type)
        {
            case (uint8_t)DPMsgType::System:
                if (header.size != sizeof(DPMsgSystem))
                    return false;
                
                break;

            default:
                break;
        }

        memcpy(data, pk->data + sizeof(header), header.size);
        return true;
    }

    /*!
        Writes a game message into buffer
        @param from ID that sends the message
        @param id ID that receives the message
        @param data data to send
        @param len Length of the data to send
    */
    void WriteGame(DPID from, DPID to, uint8_t* data, size_t len)
    {
        if (len > MAX_MSG_BUFFER)
            return; // limited for now...

        memcpy(this->data + header.size, data, len);
        header.size = len;
        header.from = from;
        header.to = to;
        header.magic = MSG_HEADER;
        header.type = (uint8_t)DPMsgType::Game;
        invalid = false;
        channel = (uint8_t)DPChannel::Normal;
        flags = 0; // not reliable
    }

    /*!
        Writes a system message into buffer
        @param from ID that sends the message
        @param id ID that receives the message
        @param msg Message to send
    */
    void WriteSystem(DPID from, DPID to, const DPMsgSystem& msg)
    {
        memcpy(data, &msg, sizeof(msg));
        header.size = sizeof(msg);
        header.from = from;
        header.to = to;
        header.magic = MSG_HEADER;
        header.type = (uint8_t)DPMsgType::System;
        invalid = false;
        channel = (uint8_t)DPChannel::Normal;
        flags = ENET_PACKET_FLAG_RELIABLE;
    }

    /*!
        Writes a chat message into buffer
        @param from ID that sends the message
        @param id ID that receives the message
        @param data string to send
        @param len Length of the string
        @note if the length is not 0 then the string is assumed
            to be with a null terminator
    */
    void WriteChat(DPID from, DPID to, const char* msg, size_t len = 0)
    {
        if (len == 0)
        {
            header.size = strlen(msg);
        }
        else
            header.size = len;

        if ((header.size + 1) > MAX_MSG_BUFFER)
            return;

        memcpy(data, msg, header.size);
        data[header.size] = 0; // null terminator!

        header.from = from;
        header.to = to;
        header.magic = MSG_HEADER;
        header.type = (uint8_t)DPMsgType::Chat;
        invalid = false;
        channel = (uint8_t)DPChannel::Chat;
        flags = ENET_PACKET_FLAG_RELIABLE;
    }
    

    /*!
        Resets the data to write into the default
    */
    void Reset()
    {
        memset(data, 0, header.size);
        header.size = 0;
    }

    /*!
        Converts a message to system
        @return Received message or NULL in case the message in invalid
    */
    inline DPMsgSystem* toSystem() const
    {
        if (header.type != (uint8_t)DPMsgType::System)
            return NULL;

        return (DPMsgSystem*)data;
    }
};

/// Message Builders
namespace MsgBuilder
{
    /*!
        Creates an authentication response ok packet
        @param id New accepted ID
        @return Newly created message
    */
    static inline DPMsg AuthOk(DPID id)
    {
        DPMsg msg;
        DPMsgSystem sysW;
        sysW.systemID = (uint8_t)DPMsgSystemType::AuthenticationRes;
        sysW.extraPeerID = id;
        sysW.extraData = (uint32_t)DPMsgAuthenticationRes::Success;
        msg.WriteSystem(DPID_SYSTEM, DPID_SYSTEM, sysW);
        return msg;
    }

    /*!
        Creates an authentication error packet
        @param err Error message
        @return Newly created message
    */
    static inline DPMsg AuthError(DPMsgAuthenticationRes err = DPMsgAuthenticationRes::UnknownError)
    {
        DPMsg msg;
        DPMsgSystem sysW;
        sysW.systemID = (uint8_t)DPMsgSystemType::AuthenticationRes;
        sysW.extraData = (uint32_t)err;
        msg.WriteSystem(DPID_SYSTEM, DPID_SYSTEM, sysW);
        return msg;
    }

    /*!
        Creates an authentication fail packet becase the lobby is full
        @return Newly created message
    */
    static inline DPMsg AuthFull()
    {
        return AuthError(DPMsgAuthenticationRes::LobbyFull);
    }

    /*!
        Creates an authentication fail packet becase the password is invalid
        @return Newly created message
    */
    static inline DPMsg AuthPass()
    {
        return AuthError(DPMsgAuthenticationRes::InvalidPassword);
    }

    /*!
        Creates a new message that indicates the disconnection of a player
        @param id ID of the player that disconnected
        @param timeout If the player has been timed out
    */
    static inline DPMsg DestroyPlayer(DPID id, bool timeout = false)
    {
        DPMsg msg;
        DPMsgSystem sysW;
        sysW.extraPeerID = id;
        sysW.systemID = timeout ? (uint8_t)DPMsgSystemType::DestroyPlayer : (uint8_t)DPMsgSystemType::TimeoutPlayer;
        msg.WriteSystem(DPID_SYSTEM, DPID_SYSTEM, sysW);
        return msg;
    }

    /*!
        Creates a new message that indicates the connection of a new player
        @param id ID of the new player
    */
    static inline DPMsg CreatePlayer(DPID id)
    {
        DPMsg msg;
        DPMsgSystem sysW;
        sysW.extraPeerID = id;
        sysW.systemID = (uint8_t)DPMsgSystemType::CreatePlayer;
        msg.WriteSystem(DPID_SYSTEM, DPID_SYSTEM, sysW);
        return msg;
    }
}

/*!
    Reference to a message
*/
using LPDPMSG = DPMsg*;
