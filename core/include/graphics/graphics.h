#pragma once

#include <SFML/Graphics/RenderTarget.hpp>

namespace core
{
/**
 * \brief DrawInterface is an interface used by the Engine to be called when the game loop is drawing elements on the screen.
 * It needs to be registered by the Engine.
 */
class DrawInterface
{
public:
    virtual ~DrawInterface() = default;
    virtual void Draw(sf::RenderTarget& renderTarget) = 0;
};

/**
 * \brief DrawImGuiInterface is an interface used by the Engine to be called when the game loop is drawing ImGui windows!
 * It needs to be registered by the Engine.
 */
class DrawImGuiInterface
{
public:
    virtual ~DrawImGuiInterface() = default;
    virtual void DrawImGui() = 0;
};
}
