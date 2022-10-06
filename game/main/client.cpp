
#include "engine/engine.h"
#include "network/client_app.h"

int main()
{
    core::Engine engine;
    game::ClientApp app;
    engine.RegisterApp(&app);

    engine.Run();
    return 0;
}