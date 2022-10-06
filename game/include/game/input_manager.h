#pragma once
#include "game/game_globals.h"

namespace game
{
/**
 * \brief GetPlayerInput gets this current player input by player index.
 * \param index is the player index
 * \return this frame current player input
 */
PlayerInput GetPlayerInput(int index);

}