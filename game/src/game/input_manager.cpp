#include "game/input_manager.h"
#include "SFML/Window/Keyboard.hpp"

namespace game
{
    
PlayerInput GetPlayerInput(int index)
{
    switch(index)
    {
    case 0:
    {
        PlayerInput clientInput1 = 0;
        clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ?
                PlayerInputEnum::PlayerInput::LEFT : PlayerInputEnum::PlayerInput::NONE);
        clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ?
                PlayerInputEnum::PlayerInput::RIGHT : PlayerInputEnum::PlayerInput::NONE);
        clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ?
                PlayerInputEnum::PlayerInput::UP : PlayerInputEnum::PlayerInput::NONE);
        clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ?
                PlayerInputEnum::PlayerInput::DOWN : PlayerInputEnum::PlayerInput::NONE);
        clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::RControl) ?
                PlayerInputEnum::PlayerInput::SHOOT : PlayerInputEnum::PlayerInput::NONE);

        return clientInput1;
    }
    case 1:
    {
        PlayerInput clientInput2 = 0;
        clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ?
                PlayerInputEnum::PlayerInput::LEFT : PlayerInputEnum::PlayerInput::NONE);
        clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ?
                PlayerInputEnum::PlayerInput::RIGHT : PlayerInputEnum::PlayerInput::NONE);
        clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ?
                PlayerInputEnum::PlayerInput::UP : PlayerInputEnum::PlayerInput::NONE);
        clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ?
                PlayerInputEnum::PlayerInput::DOWN : PlayerInputEnum::PlayerInput::NONE);
        clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ?
                PlayerInputEnum::PlayerInput::SHOOT : PlayerInputEnum::PlayerInput::NONE);
        return clientInput2;
    }
    default:
        break;
    }
    return 0;
   
}
} // namespace game
