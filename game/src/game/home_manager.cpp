#include "game/home_manager.h"
#include "game/game_manager.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
HomeManager::HomeManager(core::EntityManager& entityManager, GameManager& gameManager) :
ComponentManager(entityManager), gameManager_(gameManager)
{
}
}