/*!
    @file Server.cpp
    @brief Server implementation of DirectPlay
    @author Arves100
    @date 06/02/2023
*/
#include "StdAfx.h"
#include "Server.h"

DPServer::DPServer() : m_host(nullptr) {}

DPServer::~DPServer()
{
    for (const auto& p: m_vPlayers)
    {
        p.second->ForceDisconnect();
    }

    m_vPlayers.clear();

    if (m_host)
    {
        enet_host_destroy(m_host);
        m_host = nullptr;
    }
}

void DPServer::HandleMsg(DPMsg& msg)
{
    switch (msg.header.type)
    {
        /*
            A client has wrote something in the chat,
            notify that to us and everyone else
        */
        case (uint8_t)DPMsgType::Chat:
            Broadcast(msg);
            break;

        /*
            Notify messages to us
        */
        case (uint8_t)DPMsgType::Game:
        case (uint8_t)DPMsgType::System:
            Push(msg);
            break;            
    }
}

void DPServer::HandleUnAuthMsg(ENetPeer* peer, DPMsg& msg)
{
    if (msg.header.type != (uint8_t)DPMsgType::System)
    {
        /*
            At this stage a peer can only send a system message
            Any other peer type is threated as tampred
        */
        enet_peer_disconnect_now(peer, 0);
    }

    DPMsgSystem* sys = msg.toSystem();

    switch (sys->systemID)
    {
        /*
            We have received an authentication request,
            the game client expects a result packet with the associated ID
            to store.

            This function will also broadcast to all the current peers that a new
            client has joined the lobby/game.
        */
        case (uint8_t)DPMsgSystemType::AuthenticationReq:
        {
            // TODO: Add check for password request
            // TODO: Add check if lobby is full

            auto id = NewId();
            peer->data = reinterpret_cast<void*>(id);
            auto np = std::make_shared<DPPeer>(peer);
            np->Pair(id);

            {
                std::lock_guard lock(m_playerMtx);
                m_vPlayers.insert_or_assign(id, np);
            }

            DPMsgSystem sysW;
            sysW.systemID = (uint8_t)DPMsgSystemType::AuthenticationRes;
            sysW.extraPeerID = id;

            DPMsg res = MsgBuilder::AuthOk(id);

            np->Send(res);

            // this will broadcast all the players to the current peer
            // and the new player to all the remaining peers.
            BroadcastNewPlayer(id);

            np->AdjustTimeout();
            break;
        }

        /*
            A peer has requested a game info, we just send it
            and tell him to disconnect
        */
        case (uint8_t)DPMsgSystemType::GameInfoReq:
        {
            // TODO!

            //auto msg = DPMsg::createGameInfo(m_info);
            //DPPeer::Send(peer, msg);            
            enet_peer_disconnect(peer, 0);
            break;
        }

        default:
            // invalid packet, disconnect...
            enet_peer_disconnect_now(peer, 0);
            break;
    }
}

void DPServer::Service(uint32_t time)
{
    ENetEvent evt;
    if (enet_host_service(m_host, &evt, time))
    {
        switch (evt.type)
        {
        case ENET_EVENT_TYPE_DISCONNECT:
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
            if (evt.peer->data) // a peer that has been authenticated
            {
                auto id = reinterpret_cast<DPID>(evt.peer->data);

                auto it = m_vPlayers.find(id);

                if (it == m_vPlayers.end())
                    break;

                auto peer = it->second;

                { // mutex
                    std::lock_guard lock(m_playerMtx);
                    m_vPlayers.erase(it);
                }

                auto msg = MsgBuilder::DestroyPlayer(id, evt.type == ENET_EVENT_TYPE_DISCONNECT_TIMEOUT);
                Broadcast(msg);
            }

            // we ignore the disconnection of unauthenticated players
            break;

        case ENET_EVENT_TYPE_RECEIVE:
        {
            DPMsg msg;

            if (!msg.Read(evt.packet) || !msg.invalid)
                break; // invalid: skip this

            /*
                Normal stage:
                    - Once we know that the peer has an assigned ID, we know that
                        this peer has been authenticated to the server, so we can simply process
                        any packet like a normal TCP session would, keep in mind that the client
                        should not do any authentication step here
            */
            if (evt.peer->data) // a peer that has been authenticated
            {
                /*
                    A peer might want to send a packet to the server peer.

                    In this cases, the server not only functions as a relayer in case a peer wants
                    to talk to another peer, but the server itself is also a game client,
                    which is why it maintains an ID associated to itself.
                    Talking to the server does not need to relay to packet anywhere else.
                */
                if (msg.header.to == m_self)
                    HandleMsg(msg); // message directed to us!
                else if (msg.header.to == DPID_SYSTEM)
                {
                    /*
                        A message is directed to every peer, we just broadcast it
                    */
                    Broadcast(msg);
                }
                else
                {
                    /*
                        A peer might want to send a packet to peer C.

                        In this case:
                            Peer A -> Peer C

                        altrough Peer connects to the dedicated (host server) called Peer B

                        Peer A does not have a direct connection to Peer C because
                        this protocol is not designed to be P2P.

                        Peer A -> Peer B -> Peer C
                        in the server handles as a simple forwarder to Peer C much like a NAT
                            service would do
                    */
                    auto it = m_vPlayers.find(msg.header.to);
                    if (it == m_vPlayers.end())
                    {
                        // skip this packet because the received is invalid/discnnected
                        break;
                    }

                    it->second->Send(msg);
                }
            }
            /*
                Authentication stage:
                    - A client might send a ClientHello that contains a join request which means
                        it has to be authenticated and logged in into the server vector so
                        in this case the server can know about the existance of this client
                    - A client might send a ClientHello that contains a server information request,
                        this can be done so in case we can get information about the session without having
                        to authenticate to the Server, we still have a normal connection so we can blacklist
                        any problematic ip or host in case.
            */
            else // unauthenticated player
            {
                if (msg.header.to != DPID_INVALID || msg.header.from != DPID_INVALID)
                {
                    enet_peer_disconnect_now(evt.peer, 0); // malformed packet, we only accept anonymous messages in this stage
                    break;
                }

                HandleUnAuthMsg(evt.peer, msg);
            }
            break;
        }

        /*
            During this stage, we wait for a packet that contains the client authentication
            this is pretty much like OpenSSL ClientHello
        */
        case ENET_EVENT_TYPE_CONNECT:
            // setup handshake timeout
            enet_peer_timeout(evt.peer, HANDSHAKE_TIMEOUT, HANDSHAKE_TIMEOUT_MIN, HANDSHAKE_TIMEOUT_MAX);
            break;
        }
    }
}

void DPServer::Broadcast(DPMsg& msg)
{
    auto current = m_vPlayers;

    for (auto& pair: current)
    {
        auto p = pair.second;
        if (msg.header.from == p->GetId())
            continue;
        
        p->Send(msg);
    }

    Push(msg);
}

void DPServer::BroadcastNewPlayer(DPID id)
{
    DPMsg res = MsgBuilder::CreatePlayer(id);
    auto current = m_vPlayers;

    for (auto& pair: current)
    {
        auto p = pair.second;
        if (p->GetId() == id)
            continue;

        p->Send(res);
    }

    Push(res);
}

void DPServer::Push(DPMsg& msg)
{
    m_queue.enqueue(msg);
}
