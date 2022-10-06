#pragma once
#include <memory>
#include <network/client.h>
#include <SFML/System/Time.hpp>

#include "debug_db.h"

namespace game
{
class SimulationServer;

/**
 * \brief SimulationClient is a Client that uses simulated sockets with a direct reference to the server.
 */
class SimulationClient final : public Client
{
public:
    explicit SimulationClient(SimulationServer& server);

    void Begin() override;
    void Update(sf::Time dt) override;

    void End() override;
    void Draw(sf::RenderTarget& window) override;


    void SendUnreliablePacket(std::unique_ptr<Packet> packet) override;
    void SendReliablePacket(std::unique_ptr<Packet> packet) override;

    void ReceivePacket(const Packet* packet) override;
    
    void DrawImGui() override;
    void SetPlayerInput(PlayerInput input);
    
private:
    SimulationServer& server_;
#ifdef ENABLE_SQLITE
    DebugDatabase debugDb_;
#endif

};
}
