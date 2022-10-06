#pragma once
#include "packet_type.h"
#include "game/game_manager.h"
#include "graphics/graphics.h"

namespace game
{
/**
 * \brief Client is an interface of a player game manager and the net client interface (receive and send packets).
 * A client needs an ID which is receive by the server through a packet.
 */
class Client : public core::DrawInterface, public core::DrawImGuiInterface, public PacketSenderInterface, public core::SystemInterface
{
public:
    Client() : gameManager_(*this)
    {

    }
    virtual void SetWindowSize(sf::Vector2u windowSize)
    {
        gameManager_.SetWindowSize(windowSize);
    }

    /**
     * \brief ReceiveNetPacket is a method called by an app owning a client when receiving a packet.
     * It is the same one for simulated and network client
     * \param packet A non-owning pointer to a packet (you don't need to care about deleting it
     */
    virtual void ReceivePacket(const Packet* packet);

    void Update(sf::Time dt) override;
protected:

    ClientGameManager gameManager_;
    ClientId clientId_ = INVALID_CLIENT_ID;
    float pingTimer_ = -1.0f;
    float currentPing_ = 0.0f;
    static constexpr float pingPeriod_ = 0.3f;

    float srtt_ = -1.0f;
    float rttvar_ = 0.0f;
    float rto_ = 1.0f;
    static constexpr float k = 4.0f;
    static constexpr float g = 100.0f;
    static constexpr float alpha = 1.0f/8.0f;
    static constexpr float beta = 1.0f/4.0f;
};
}
