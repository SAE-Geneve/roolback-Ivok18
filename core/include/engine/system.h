#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

namespace core
{
/**
 * \brief SystemInterface is an interface to a game system that needs to begin, update and end by the Engine.
 * It needs to be registered by the Engine.
 */
class SystemInterface
{
public:
    virtual ~SystemInterface() = default;
    virtual void Begin() = 0;
    virtual void Update(sf::Time dt) = 0;
    virtual void End() = 0;
};

/**
 * \brief OnEventInterface is an interface to a class that needs to read events from the Engine.
 * It needs to be registered in the Engine.
 */
class OnEventInterface
{
public:
    virtual ~OnEventInterface() = default;
    virtual void OnEvent(const sf::Event& event) = 0;
};

}
