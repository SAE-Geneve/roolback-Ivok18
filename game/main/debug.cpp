#include "network/simulation_app.h"

#include "engine/engine.h"


int main()
{
    core::Engine engine;
    game::SimulationApp app;
    engine.RegisterApp(&app);

    engine.Run();
    return 0;
}