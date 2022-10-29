#include "game/healthbar_manager.h"
#include "game/game_manager.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
	HealthBarManager::HealthBarManager(core::EntityManager& entityManager, GameManager& gameManager) :
		ComponentManager(entityManager), gameManager_(gameManager)
	{
	}

}