#pragma once
#include <memory>

#include "packet_type.h"
#include "engine/system.h"
#include "game/game_globals.h"
#include "game/game_manager.h"

namespace game
{
/**
 * \brief Server is an interface to a network or simulated server. It owns a simple GameManager.
 */
class Server : public PacketSenderInterface, public core::SystemInterface
{
protected:

    virtual void SpawnNewPlayer(ClientId clientId, PlayerNumber playerNumber) = 0;
    /**
     * \brief ReceiveNetPacket is a method that is called when the Server receives a Packet from a Client.
     * \param packet is the received Packet.
     */
    virtual void ReceivePacket(std::unique_ptr<Packet> packet);

    //Server game manager
    GameManager gameManager_;
    PlayerNumber lastPlayerNumber_ = 0;
    std::array<ClientId, maxPlayerNmb> clientMap_{};

};
}
