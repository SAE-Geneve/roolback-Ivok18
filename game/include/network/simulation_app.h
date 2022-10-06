#pragma once
#include "client.h"
#include "simulation_client.h"
#include "simulation_server.h"
#include "engine/app.h"
#include "game/game_globals.h"

#include <SFML/Graphics/RenderTexture.hpp>

namespace game
{
/**
 * \brief SimulationApp is an application that owns SimulationClient and SimulationServer.
 */
class SimulationApp final : public core::App
{
public:
    SimulationApp();

    void Begin() override;

    void Update(sf::Time dt) override;

    void End() override;

    void DrawImGui() override;

    void Draw(sf::RenderTarget& window) override;

    void OnEvent(const sf::Event& event) override;
private:
    std::array<std::unique_ptr<SimulationClient>, maxPlayerNmb> clients_;
    std::array<sf::RenderTexture, maxPlayerNmb> clientsFramebuffers_;
    SimulationServer server_;
    sf::Sprite screenQuad_;
    sf::Vector2u windowSize_;
};
}
