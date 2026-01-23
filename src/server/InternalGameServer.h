#ifndef TICTACTOEOVERLAN_INTERNALGAMESERVER_H
#define TICTACTOEOVERLAN_INTERNALGAMESERVER_H

#include <atomic>
#include <map>
#include <vector>
#include <winsock2.h>

#include "ClientContext.h"
#include "../common/LongLongRollingAverage.h"
#include "../common/NetworkProtocol.h"

#pragma comment(lib, "Ws2_32.lib")

/**
 * @brief The authoritative server logic for the game.
 * <br> Runs on a dedicated thread hosted by one of the clients.
 * <br> Responsibilities include:
 * <br> 1. Accepting new connections (TCP).
 * <br> 2. Managing the main game loop (Tick rate).
 * <br> 3. Enforcing game rules and state synchronization.
 * <br> 4. Broadcasting updates to all connected clients.
 */
class InternalGameServer {
    std::atomic<bool> keepRunning;
    SOCKET listenSocket;
    std::atomic<long long> tick = 0;
    std::atomic<long long> lastTickTime = 0;
    LongLongRollingAverage avgTickTime{100}; //Thread-safe with mutex inside, so no need for atomic

    // std::map<uint8_t, ClientContext> clients;
    std::vector<ClientContext> clients;
    uint8_t nextPlayerId = 1;
    uint8_t hostingPlayerId = 0;
    std::vector<PieceType> availablePieces;

    //Game State
    BoardData boardData;
    std::vector<Move> moves;
    // The clientContexts also hold player data and state

public:
    InternalGameServer() : keepRunning(false),
                           listenSocket(INVALID_SOCKET),
                           boardData({{}, 3, 3, 0, 1}) {
    };

    /**
     * @brief Starts the server loop.
     * <br> Binds to the specified port and enters the main `while(keepRunning)` loop.
     *
     * @param port The port number to listen on.
     */
    void start(int port);

    /**
     * @brief Signals the server loop to terminate.
     * <br> Sets `keepRunning` to false.
     */
    void stop();

    //getters - for debug purposes
    long long getTick();

    long long getLastTickTime();

    double getAvgTickTime();

    uint8_t getNextPlayerId() const;

    uint16_t getCurrentTurn() const;

    uint8_t getHostingPlayerId() const;

    std::tuple<uint8_t, uint8_t> getBoardSettings();

    std::vector<PieceType> getAllAvailablePieces();

    std::vector<Player> getPlayers() const;

    std::vector<Move> getMoves();

private:
    /**
     * @brief Accepts and processes a new connection.
     * <br> If a client connects, creates a new `ClientContext`, assigns an ID, and sends `SERVER_HELLO`.
     */
    void handleNewConnection();

    /**
     * @brief Reads incoming data from a specific client.
     * <br> Appends data to the client's `receiveBuffer`.
     * <br> If a full packet is assembled, calls `processPacket`.
     *
     * @param client The client connection to poll.
     */
    void handleClientData(ClientContext &client);

    /**
     * @brief Retrieves and removes the next available piece type from the pool.
     *
     * @return The piece type.
     */
    PieceType getFirstAvailablePiece();

    /**
     * @brief Determines whose turn it is based on the round/turn counters.
     *
     * @return The ID of the acting player.
     */
    uint8_t getNextActingPlayerId() const;

    /**
     * @brief The core logic dispatcher.
     * <br> Switches on `PacketType` and executes the corresponding game logic.
     *
     * @param client The client who sent the packet.
     * @param type The type of packet received.
     * @param payload The raw byte data of the packet body.
     */
    void processPacket(ClientContext &client, PacketType type, std::vector<char> &payload);

    /**
     * @brief Processes the SETUP_REQ packet.
     *
     * @param client The client from which we received the packet.
     * @param packet The parsed SetupReqPacket packet.
     */
    void handleSetupRequestPacket(ClientContext &client, const SetupReqPacket *packet);

    /**
     * @brief Processes the SETTINGS_CHANGE_REQ packet.
     *
     * @param packet The parsed SettingsChangeReqPacket packet.
     */
    bool handleSettingsChangeRequestPacket(const SettingsChangeReqPacket *packet);

    /**
     * @brief Processes the GAME_START_REQ packet.
     *
     * @param packet The parsed GameStartRequestPacket packet.
     */
    bool handleGameStartRequestPacket(const GameStartRequestPacket *packet);

    /**
     * @brief Processes the MOVE_REQ packet.
     *
     * @param client The client from which we received the packet.
     * @param packet The parsed MoveRequestPacket packet.
     */
    bool handleMoveRequestPacket(ClientContext &client, const MoveRequestPacket *packet);

    /**
     * @brief Sends a structured packet to a specific client.
     *
     * @param sock The target socket.
     * @param type The packet type identifier.
     * @param data The payload struct.
     */
    template<typename T>
    static void sendPacket(SOCKET sock, PacketType type, const T &data);

    /**
     * @brief Sends a packet to ALL connected clients.
     *
     * @param type The packet type identifier.
     * @param data The payload struct.
     */
    template<typename T>
    void broadcastPacket(const PacketType type, const T &data);
};


#endif //TICTACTOEOVERLAN_INTERNALGAMESERVER_H
