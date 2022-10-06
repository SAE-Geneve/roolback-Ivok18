#pragma once

#ifdef ENABLE_SQLITE
#include "network/packet_type.h"
#include "game/physics_manager.h"

#include <string_view>
#include <condition_variable>
#include <thread>
#include <vector>

struct sqlite3;

namespace game
{

struct DbPhysicsState
{
    std::array<PhysicsState, maxPlayerNmb> serverStates{};
    std::array<PhysicsState, maxPlayerNmb> localStates{};
    Frame lastLocalValidateFrame{};
    Frame validateFrame{};
};

class DebugDatabase
{
public:
    void Open(std::string_view path);
    void StorePacket(const PlayerInputPacket* inputPacket);
    void StorePhysicsState(const DbPhysicsState& physicsState);
    void Close();
private:
    void Loop();
    void CreateTables() const;
    sqlite3* db = nullptr;
    std::atomic<bool> isOver_ = false;
    std::thread t_;
    mutable std::mutex m_;
    std::condition_variable cv_;
    std::vector<std::string> commands_;
};

}
#endif