#include "network/debug_db.h"

#ifdef ENABLE_SQLITE

#include "utils/log.h"
#include "utils/conversion.h"


#include <sqlite3.h>
#include <fmt/format.h>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

#include <filesystem>

namespace fs = std::filesystem;


namespace game
{

static int callback([[maybe_unused]] void* NotUsed, int argc, char** argv, char** azColName)
{
    for (int i = 0; i < argc; i++) {
        core::LogDebug(fmt::format("{} = {}", azColName[i], argv[i] ? argv[i] : "NULL"));
    }
    return 0;
}

void DebugDatabase::Open(std::string_view path)
{
    if (fs::exists(path))
    {
        fs::remove(path);
    }
    const auto rc = sqlite3_open(path.data(), &db);
    if (rc != SQLITE_OK)
    {
        core::LogError(fmt::format("Can't open database: {}\n", sqlite3_errmsg(db)));
        sqlite3_close(db);
        db = nullptr;
    }
    CreateTables();
    t_ = std::thread{ &DebugDatabase::Loop, this };
    t_.detach();
}

void DebugDatabase::StorePacket(const PlayerInputPacket* inputPacket)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const PlayerNumber playerNumber = inputPacket->playerNumber;
    const auto frame = core::ConvertFromBinary<Frame>(inputPacket->currentFrame);
    const PlayerInput input = inputPacket->inputs[0];

    auto query = fmt::format("INSERT INTO inputs (player_number, frame, up, down, left, right, shoot) VALUES({}, {}, {}, {}, {}, {},  {});",
        playerNumber,
        frame,
        (input & PlayerInputEnum::UP) == PlayerInputEnum::UP,
        (input & PlayerInputEnum::DOWN) == PlayerInputEnum::DOWN,
        (input & PlayerInputEnum::LEFT) == PlayerInputEnum::LEFT,
        (input & PlayerInputEnum::RIGHT) == PlayerInputEnum::RIGHT,
        (input & PlayerInputEnum::SHOOT) == PlayerInputEnum::SHOOT);

    {
        std::lock_guard lock(m_);
        commands_.push_back(query);
    }

    cv_.notify_one();
}

void DebugDatabase::StorePhysicsState(const DbPhysicsState& physicsState)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

    std::string query = "INSERT INTO physics_state (local_frame, validate_frame ";
    std::string arguments;
    std::string values = fmt::format(" VALUES ({}, {}", physicsState.lastLocalValidateFrame, physicsState.validateFrame);
    for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
    {
        arguments += fmt::format(",state_p{}_local, state_p{}_server ", playerNumber + 1, playerNumber + 1);
        values += fmt::format(",{},{}", physicsState.localStates[playerNumber], physicsState.serverStates[playerNumber]);
    }
    query += arguments + ") " + values + " );";
    {
        std::lock_guard lock(m_);
        commands_.push_back(query);
    }

    cv_.notify_one();
}

void DebugDatabase::Close()
{
    isOver_.store(true, std::memory_order_release);
    cv_.notify_one();

    //t_.join();
    if (db != nullptr)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}

void DebugDatabase::Loop()
{
    std::unique_lock lock(m_);
    while (!isOver_.load(std::memory_order_acquire))
    {
        while (!commands_.empty())
        {
            if (isOver_.load(std::memory_order_acquire))
            {
                break;
            }
#ifdef TRACY_ENABLE
            ZoneNamedN(sqlExecuteCommand, "SQL Execute Command", true);
#endif
            std::string command = std::move(commands_[0]);
            commands_.erase(commands_.begin());
            lock.unlock();
            /* Execute SQL statement */

            char* zErrMsg = nullptr;
            const auto rc = sqlite3_exec(db, command.c_str(), callback, nullptr, &zErrMsg);

            if (rc != SQLITE_OK) {
                core::LogError(fmt::format("SQL error with storing input: {}", zErrMsg));
                sqlite3_free(zErrMsg);
            }
            lock.lock();
        }
        if (isOver_.load(std::memory_order_acquire))
            break;
        cv_.wait(lock);
    }
    isOver_ = false;
}

void DebugDatabase::CreateTables() const
{
    //playerNumber, frame, up, down, left, right, shoot

    const auto createInputTable = "CREATE TABLE inputs ("\
        "input_id INTEGER PRIMARY KEY,"\
        "player_number INTEGER NOT NULL,"\
        "frame INTEGER NOT NULL,"\
        "up INTEGER NOT NULL,"\
        "down INTEGER NOT NULL,"\
        "left INTEGER NOT NULL,"\
        "right INTEGER NOT NULL,"\
        "shoot INTEGER NOT NULL);";

    /* Execute SQL statement */

    char* zErrMsg = nullptr;
    const auto rc = sqlite3_exec(db, createInputTable, callback, nullptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        core::LogError(fmt::format("SQL error while creating table: {}", zErrMsg));
        sqlite3_free(zErrMsg);
    }

    std::string createPhysicsStateTable = "CREATE TABLE physics_state ("\
        "phys_id INTEGER PRIMARY KEY,"\
        "local_frame INTEGER NOT NULL,"\
        "validate_frame INTEGER NOT NULL"\
        ;
    for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
    {
        createPhysicsStateTable += fmt::format(",state_p{}_local INTEGER NOT NULL, state_p{}_server INTEGER NOT NULL", playerNumber + 1, playerNumber + 1);
    }
    createPhysicsStateTable += ");";
    zErrMsg = nullptr;
    const auto rc2 = sqlite3_exec(db, createPhysicsStateTable.data(), callback, nullptr, &zErrMsg);
    if (rc2 != SQLITE_OK) {
        core::LogError(fmt::format("SQL error while creating table: {}", zErrMsg));
        sqlite3_free(zErrMsg);
    }
}
}

#endif