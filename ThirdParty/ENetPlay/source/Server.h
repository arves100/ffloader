/*!
    @file Server.h
    @brief Server implementation of DirectPlay
    @author Arves100
    @date 06/02/2023
*/
#pragma once

#include "Msg.h"
#include "Peer.h"

/*!
    This class rapresents a reimplementation of a DirectPlay server
    by using ENet
*/
class DPServer final
{
public:
    /*!
        Default constructor
    */
    explicit DPServer();

    /*!
        Default deconstructor
    */
    virtual ~DPServer();

    /*!
        Services the enet host
        @param timeout Timeout time
        @note this function is usually called by the Service thread, but it might
            be called by the game thread in syncronous operations.
    */
    void Service(uint32_t timeout);

private:
    /*!
        This function handles a message received from an authenticated peer
        @param msg Message received
    */
    void HandleMsg(DPMsg& msg);

    /*!
        This function handles a message received from a peer that
        has not been authenticated yet, this may mean a new connection
        or a game info request
        @param peer Peer that sent the message
        @param msg Message received
    */
    void HandleUnAuthMsg(ENetPeer* peer, DPMsg& msg);

    /*!
        Broadcast a message to all the peers currently connected
        to the server (including the server itself)
        @param msg Message to broadcast
        @note The sender will not receive the message broadcasted
    */
    void Broadcast(DPMsg& msg);

    /*!
        This function broadcasts the event that a new player
        has been joined the lobby/game to all the connected peers
        including ourself.
        @param id ID of the newly connected peer
    */
    void BroadcastNewPlayer(DPID id);

    /*!
        Allocates a new id for the newly connected peer
        @return A new ID
    */
    DPID NewId();

    /*!
        This function pushes a message to the message queue map,
        the message will be ready to be dispatched by the game thread
        (basically by the server)
        @param msg Message to push
        @note this function is called only by the service thread
    */
    void Push(DPMsg& msg);

    /// ENet host
    ENetHost* m_host;

    /// Player map (this does not contain the server peer)
    ankerl::unordered_dense::map<DPID, std::shared_ptr<DPPeer>> m_vPlayers;

    /// Message queue map
    moodycamel::ConcurrentQueue<DPMsg> m_queue;

    /// The ID of the server peer
    DPID m_self;

    /// A mutex that locks player map write access
    std::mutex m_playerMtx;
};
