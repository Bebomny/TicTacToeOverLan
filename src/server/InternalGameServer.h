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
    InternalGameServer():
    keepRunning(false),
    listenSocket(INVALID_SOCKET),
    boardData({{}, 3, 3, 0, 1}) {};

    void start(int port);
    void stop();

    //getters - for debug purposes
    long long getTick();
    long long getLastTickTime();
    double getAvgTickTime();
    uint8_t getNextPlayerId();
    uint16_t getCurrentTurn();
    uint8_t getHostingPlayerId();
    std::tuple<uint8_t, uint8_t> getBoardSettings();
    std::vector<PieceType> getAllAvailablePieces();
    std::vector<Player> getPlayers();

private:
    void handleNewConnection();
    void handleClientData(ClientContext& client);
    void disconnectClient(size_t index);

    PieceType getFirstAvailablePiece();
    uint8_t getNextActingPlayerId();

    void processPacket(ClientContext& client, PacketType type, std::vector<char>& payload);

    template<typename T>
    static void sendPacket(SOCKET sock, PacketType type, const T &data);

    template<typename T>
    void broadcastPacket(const PacketType type, const T &data);
};


#endif //TICTACTOEOVERLAN_INTERNALGAMESERVER_H
