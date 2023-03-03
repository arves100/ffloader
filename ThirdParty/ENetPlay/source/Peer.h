/*!
    @file Peer.h
    @brief Peer implementation of DirectPlay
    @author Arves100
    @date 06/02/2023
*/
#pragma once

#define HANDSHAKE_TIMEOUT 500 // 0.5 seconds
#define HANDSHAKE_TIMEOUT_MIN 500 // 0.5 seconds minimum timeout
#define HANDSHAKE_TIMEOUT_MAX 1000 // 1 second of timeout

#define TIMEOUT 300 // 300ms
#define TIMEOUT_MIN 10 // 10ms
#define TIMEOUT_MAX 2000 // 5 second

#define IP_MAX_NUM 60

/*!
    This class rapresents a ENet peer used by the server or
    the client
*/
class DPPeer final
{
public:
    /*!
        Default constructor
    */
    explicit DPPeer(ENetPeer* peer) : m_peer(peer), m_id(0), m_bPaired(false)
    {
    }

    /*!
        Default distructor
    */
    virtual ~DPPeer()
    {
        if (m_peer)
        {
            enet_peer_reset(m_peer);
            m_peer = nullptr;
        }
    }

    /*!
        Forces the disconnect of a peer
        @note this function DOES NOT FLUSH network data,
            use with caution!
    */
    void ForceDisconnect()
    {
        enet_peer_disconnect_now(m_peer, 0);
    }

    /*!
        Pair the peer
        @param id ID of the peer
    */
    void Pair(DPID id)
    {
        m_id = id;
        m_bPaired = true;
    }

    /*!
        Gets the IP of the peer
        @return The IP of the peer
    */
    std::string GetIP() const
    {
        char addr[IP_MAX_NUM];
        enet_address_get_ip(&m_peer->address, addr, sizeof(addr));
        return addr;
    }

    /*!
        Gets the port of the peer
        @return The port of the peer
    */
    uint16_t GetPort() const { return m_peer->address.port; }

    /*!
        Checks if the peer has been paired with a client or server
        @return true in case the peer was paired, otherwise false
    */
    bool IsPaised() const { return m_bPaired; }

    /*!
        Gets the peer id
        @return Peer id
    */
    DPID GetId() const { return m_id; }

    /*!
        Sends a message to the peer
        @param msg Message to send 
        @note This is called by the enet service thread, this 
            function really sends a  message to the network.
    */
    int Send(DPMsg& msg)
    {
        return Send(m_peer, msg);
    }

    /*!
        This function sends a message to a peer
        @param peer Peer where the message should be send
        @param msg Message to send
        @note This function really sends a message to the network!
    */
    static int Send(ENetPeer* peer, DPMsg& msg)
    {
        auto pk = enet_packet_create(&msg.header, msg.header.size + sizeof(msg.header), msg.flags);
        if (!pk)
            return -2;

        memcpy(pk->data + sizeof(msg.header), msg.data, msg.header.size);
        return enet_peer_send(peer, msg.channel, pk);
    }

    /*!
        This function adjusts the timeout of the peer after it
            has been authenticated
    */
    void AdjustTimeout()
    {
        enet_peer_timeout(m_peer, TIMEOUT, TIMEOUT_MIN, TIMEOUT_MAX);
    }

private:
    /// ENet peer
    ENetPeer* m_peer;

    /// Peer id
    DPID m_id;

    /// Check if the client and server was paired
    bool m_bPaired;
};
