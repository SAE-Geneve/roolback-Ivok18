#pragma once

#include "engine/system.h"
#include "graphics/graphics.h"

namespace core
{
/**
 * \brief App is an interface for applications that need to init/update/destroy, draw and get event from OS.
 */
class App : public DrawInterface, public DrawImGuiInterface, public SystemInterface, public OnEventInterface
{

};
}
