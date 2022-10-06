#include "engine/engine.h"
#include "utils/log.h"

#include "engine/system.h"
#include "graphics/graphics.h"

#include "engine/globals.h"

#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "utils/assert.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace core
{
void Engine::Run()
{
    Init();
    sf::Clock clock;
    while (window_->isOpen())
    {
        try
        {
            const auto dt = clock.restart();
            Update(dt);
#ifdef TRACY_ENABLE
            FrameMark;
#endif
        }
        catch ([[maybe_unused]] const AssertException& e)
        {
            LogError("Exit with exception");
            window_->close();
        }
    }
    Destroy();
}

void Engine::RegisterApp(App* app)
{
    RegisterSystem(app);
    RegisterDraw(app);
    RegisterOnEvent(app);
    RegisterDrawImGui(app);
}

void Engine::RegisterSystem(SystemInterface* system)
{
    systems_.push_back(system);
}

void Engine::RegisterOnEvent(OnEventInterface* onEventInterface)
{
    eventInterfaces_.push_back(onEventInterface);
}

void Engine::RegisterDraw(DrawInterface* drawInterface)
{
    drawInterfaces_.push_back(drawInterface);
}

void Engine::RegisterDrawImGui(DrawImGuiInterface* drawImGuiInterface)
{
    drawImGuiInterfaces_.push_back(drawImGuiInterface);
}

void Engine::Init()
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    window_ = std::make_unique<sf::RenderWindow>(sf::VideoMode(windowSize.x, windowSize.y), "Rollback Game");
    const bool status = ImGui::SFML::Init(*window_);
    if(!status)
    {
        LogError("Could not init ImGui-SFML");
    }
    for(auto* system : systems_)
    {
        system->Begin();
    }
}

void Engine::Update(sf::Time dt) const
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    sf::Event e{};
    while (window_->pollEvent(e))
    {
        ImGui::SFML::ProcessEvent(e);
        switch (e.type)
        {
        case sf::Event::Closed:
            window_->close();
            break;
        case sf::Event::Resized:
        {
            sf::FloatRect visibleArea(0, 0, static_cast<float>(e.size.width), static_cast<float>(e.size.height));
            window_->setView(sf::View(visibleArea));
            break;
        }
        default:
            break;
        }
        for(auto* eventInterface : eventInterfaces_)
        {
            eventInterface->OnEvent(e);
        }
    }
    for(auto* system : systems_)
    {
        system->Update(dt);
    }
    ImGui::SFML::Update(*window_, dt);
    window_->clear(sf::Color::Black);

    for(auto* drawInterface : drawInterfaces_)
    {
        drawInterface->Draw(*window_);
    }
    for(auto* drawImGuiInterface : drawImGuiInterfaces_)
    {
        drawImGuiInterface->DrawImGui();
    }
    ImGui::SFML::Render(*window_);

    window_->display();
}

void Engine::Destroy()
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (auto* system : systems_)
    {
        system->End();
    }
    window_ = nullptr;
}
} // namespace core
