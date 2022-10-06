/*! \mainpage GPR5100 - Rollback Game Index Page
 *
 * \section intro_sec Introduction
 *
 * This is the documentation of the GPR5100 - Rollback Game sommative project. It is a simple asteroid-like online multiplayer game using rollback mechanisms for client-side prediction and server authorative validation.
 *
 * \section install_sec Installation
 * To start working on the project, you are required CMake and vcpkg.
 * \subsection windows Windows
 * On Windows, you can use Visual Studio 2022 that supports C++20 pretty well.
 * \subsection ubuntu Ubuntu
 * Tested on Ubuntu 20.04 with gcc-10 and g++-10 and Ubuntu 22.04. To install the required packages for vcpkg:
 * \code
 * sudo apt install autoconf libtool libsfml-dev libxrandr-dev libudev-dev
 * \endcode
 * In command-line, you can setup cmake and make with:
 * \code
 * cmake -DCMAKE_TOOLCHAIN_FILE=<VCPKG_HOME>/scripts/buildsystems/vcpkg.cmake .. //-DCMAKE_BUILD_TYPE= Debug/RelWithDebInfo/Release
 * make -j 4
 * \endcode
 * \section ecs ECS implementation
 * ECS or Entity-Component-System is a pattern of structuring game world objects and their components. In our purpose, it allows to simply replicate the game data as each component is stored by type. 
 * \subsection entity_manager Entity Manager
 * The core::EntityManager is the main class of our ECS implementation. From the EntityManager, you can get if an Entity exists, if it contains a certain Component and you can destroy it. 
 * 
 * The implementation is pretty straightforward. Using bitwise operation, we store the Component type in a unique EntityMask. This saves a lot of memory and is pretty fast to get the result.
 * 
 * However, you cannot get access to the specific Component of an Entity. For that, you need to get access to its ComponentManager.
 * 
 * Here are the typical use-case of the EntityManager:
 * \code
 * const auto entity = entityManager_.CreateEntity();
 * //...
 * if(entityManager_.HasComponent(entity, static_cast<core::EntityMask>(core::ComponentType::TRANSFORM)))
 * {
 * //...
 * entityManager_.DestroyEntity(entity);
 * \endcode
 * \subsection component_manager Component Manager
 * The core::ComponentManager is a template class that automatically generates a std::vector of the given component type T. It requires an core::EntityManager, such that when adding or removing a component, the core::ComponentManager can notify the core::EntityManager of the change.
 * Here are the typical use-case:
 * \code
 * //With core::Entity entity and core::ComponentManager spriteManager_
 * spriteManager_.AddComponent(entity);
 * //...
 * auto& sprite = spriteManager_.GetComponent(entity);
 * //... Do things with sprite
 * spriteMManager_.RemoveComponent(entity);
 * \endcode
 * \subsection sprite_manager Sprite Manager
 * The core::SpriteManager is a core::ComponentManager that owns the sprites in the game. Sprites are using sf::Sprite from SFML to draw on the window.
 * 
 * Sprite draw ordering works with core::Entity ordering. It means that the background should be the first entity to spawn because it will drawn first, therefore behind the next sprites.
 * 
 * Sprites are centered on the position of the core::PositionManager by default.
 * \subsection physics_manager Physics Manager
 * The game::PhysicsManager is a class that contains two core::ComponentManager:
 * - game::BodyManager owns the game::Body (or rigid bodies) of the physics engine.
 * - game::BoxManager owns the game::Box (or box colliders) of the physics engine.
 * 
 * The Physics Engine is a REALLY simple implementation of basic box triggering. 
 * 
 * When a trigger occurs, the game::PhysicsManager will call the OnTrigger method all its game::OnTriggerInterface. To register a game::OnTriggerInterface, you have to give it to the RegisterTriggerListener method.
 * \subsection transform_manager Transform Manager
 * The core::TransformManager is a class that contains three core::ComponentManager:
 * - core::PositionManager owns the positions in meter of all entities, both used in physics and graphics (converted to pixel with pixelToMeter).
 * - core::ScaleManager owns the scaling ratios of all entities. This only applies to the graphics part of the game, NOT the physics part.
 * - core::RotationManager owns the angles (in core::Degree) of all entites. This only applies to the graphics part of the game, NOT the physics.
 * \section netcode Netcode
 * This project netcode is pretty simple, but should work for any simple game project. 
 * \subsection server_connection Connecting to the server
 * When a client wants to connect to the server to be able to join a game, they will have to connect both in TCP and UDP. This is the step-by-step:
 * 1. The Client connects to the TCP IP address and port and sends a JOIN packet
 * 2. The Server answers with a JOIN_ACK packet containing the UDP port.
 * 3. The Client sends a JOIN packet on the UDP channel.
 * 4. The Server answers with a JOIN_ACK on the UDP channel. The Client is then a valid connected client. 
 * \subsection spawn_player Spawn player
 * When a new player client connects to the server, their player character is also spawned as well. To spawn all player characters when a new client connects, the server sends the game::SpawnPlayerPacket from all current player characters to all player clients. Spawn positions, rotations and colors are hardcoded in the <a href="game__globals_8h.html">game_globals.h</a> header file.
 * \subsection start_game Starting the game
 * When all players are connected, the server automatically send a game::StartGamePacket to each player through the TCP channel. Each client will then wait about <a href="game__globals_8h.html">game::startDelay</a> milliseconds before starting their game session.
 * \subsection send_input Sending player inputs
 * Each frame, the game sends the current player inputs (game::PlayerInputPacket), as well as the last <a href="game__globals_8h.html">game::maxInputNmb</a> inputs in an UDP packet.
 * \subsection validate_frame Validating the frame
 * When the server finally receives all the player inputs for a specific frame, it will automatically validate the specific frame and will update its lastValidateFrame_ to the new specific frame. It will then sends a game::ValidateFramePacket to all clients.
 * 
 * On the client side, when receiving a game::ValidateFramePacket, the client will calculate the frame physics status and check that the result is the same as the server one. If it is not the case, there is desynchronisation and the game must end!
 * \subsection ping Ping
 * It is always important to know the current round trip time between a client and a server. The ping system is pretty simple. The client sends a PING Packet (game::PingPacket) to the server containing the current time and the server sends the same Packet back. When the client gets the game::PingPacket back, it can calculate the time it took for the Packet to do the round trip (RTT).
 * 
 * We then use TCP Retransmission Timer to calculate srtt and rttvar to get an idea of the average and variability of the packet.
 * \subsection win_game Win game
 * When the server validates the frame where a win/lose condition occurs, it sends a game::WinGamePacket on a reliable channel to all the clients with the info on the winning player. This allows all clients to stop their game loop and show an ending message (You won! or The other player won!).
 * \subsection net_simulation Net Simulation
 * Instead of always setting up a server and clients, this project allows to use a single executable with a network simulation. It does not use SFML sockets and packets, but simulates delays and packet loss (which can be customized in the UI, image below). It uses the same server and client game managers so you do not need to change anything. It also allows to test the limits of your netcode by increasing the delay estimated time as well as variability without requiring a specific network for that. It is available using the "debug" executable.
 * \image html simulation_ui.png
 * \section rollback Rollback mechanisms
 * \subsection rollback_how How the rollback works?
 * At any time, each client have the game world state of two points in time:
 * - Last Validated Frame
 * - Current Frame (except the server)
 * The server only stores the last validated frame's physics state.
 * \subsection current_frame Client Current Frame
 * To allow real time illusion, the client controls its player character in real time without waiting the validation of the server. For other clients, the rollback manager will simply repeat the last received inputs.
 * 
 * After receiving other clients inputs, the rollback manager will run all the FixedUpdate methods between the last validated frame and the current frame before running the new current frame.
 * \subsection physics_checksum Validating a Frame
 * When validating a frame, the server calculates the new physics state and will then generate a checksum (a 16-bit number) per player of the player character positions, rotations and velocities (linear and angular). This number is sent in the game::ValidateStatePacket with the validated frame index.
 * 
 * The clients will then validate the frame by calculating the physics state up to the server validated frame and will then compare the checksums values. If the values differ, it is the end of the game, because the physics state of the client is in desync, meaning that the physics simulation was not determinist compare to the other process/host.
 * \subsection destroy_entity Create And Destroy Entities
 * On the client side, due to the delta time between the last validate frame from the server and the current frame on the client, we cannot be sure that an entity is actually created or destroyed when creating or destroying an entity. It means that we have to wait for the server to confirm the frame where an entitiy is created or destroyed, before actually create or destroy the entity.
 * 
 * For entity creation, it is a rather easy problem to solve. We just have to store when a entity is created (game::CreatedEntity struct). Before calculating a new current frame from the last validated frame or when validating a frame, we just check this frame time with the last validated frame and if it is younger, we simply destroy the entity (because it will be created again when simulating).
 * 
 * For entity destruction, the chosen solution do not actually destroy the entity. We simply add a DESTROY flag in the core::EntityManager (like an empty Component) when simulating a new frame. If we are validating a frame, we simply destroy the entity. This means that the FixedUpdate methods have to check both if an entity exists and that there is no DESTROY component. 
 * \section game_manager GameManager
 * The game is managed in the game::GameManager. However, depending if the application is client- or server-side, the requirements on the GameManager are completely different.
 * \subsection server_game_manager Server GameManager
 * Due to the nature of the server-side, the server game::GameManager does not need to care about the graphical part of the game (managed in the Update method), only the physical part (mostly managed in the FixedUpdate). It does not have an Update method, as the physics state validation is only done when receiving all the player inputs of a certain frame. The server only reacts to received packets to move forward in the game. Obviously, the server is always back in the game past compared to the clients, but it is authorative.
 * 
 * For an event to only happen on the server side, one must implement it in the game::Server class (NOT in the game::GameManager, because it will be used in the game::ClientGameManager as well).
 * \subsection client_game_manager Client GameManager
 * The game::ClientGameManager inherits from the server game::GameManager and extends its features with graphical interface and real time client requirements. It means that like the server, it manages the receiving inputs, but at the same time, it also update the graphical part of the game in the Update method while updating the rollbacked phyiscal state in a continuous FixedUpdate way (it does not wait for other player inputs to move forward in time for a true real time illusion).
 * 
 * Event happening in the game::ClientGameManager only happens on the client-side, no need to implement them in the game::Client.
 * \section sqlite SQLite
 * To debug efficiently the missbehavior of the netcode, the framework is providing a SQLite database allowing to review the last session. To use it, please enable ENABLE_SQLITE_STORE in your CMake options. You can use DB Browser for SQLite to open the databases created in the binaries folder. Each client will create its own database using its core::ClientId (for example Client85.db for a client who ClientId is 85).
 * \subsection input_dbg Input debugging
 * The SQLite database stores the input of all players per frame on each client and servers. Inputs are stored when the local client makes an input or when we receive a game::PlayerInputPacket from a remote client. The database will store:
 * - The frame when the input occured
 * - The player number who did the input
 * - The actual input (for the asteroid-like game, it is up, down, left, right and shoot).
 * \subsection physics_state Physics State debugging
 * The SQLite database stores the physics state of all players when receiving a frame confirmation from the server. The database stores those data:
 * - local_frame, the current frame on the client side.
 * - validate_frame, the validate frame from the server.
 * - For each player: state_pN_local, state_pN_server (N being the player number) are the physics checksums.
 * Those data allows to debug on all clients where the client desyncs from the server.
 * \section miscellaneous Miscellaneous
 * \subsection angle Angles
 * Please use the provided core::Degree class if you need angles. It allows to use the trigonometric functions (core::Sin, core::Cos, core::Tan, core::Asin, core::Acos, core::Atan, core::Atan2) without worrying about conversions between degrees and radians.
 * \subsection assertion Assertion
 * To avoid crashes at random places due to invalid values, the core library allows to declare assertations (gpr_assert and gpr_warn) that can catch invalid values. For that to happen, you need to enable Gpr_Assert on the CMake options. When the expression is false, the assertion will throw an core::AssertException that will quietly close the application (if you need to debug the stack data, please enable Gpr_Abort in the CMake options, it will use std::abort instead). You can also have a warning assertation, a warning that is not that important that you can decide if you want to abort or not (with the CMake option Gpr_Exit_On_Warning).
 * 
 */