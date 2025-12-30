#ifndef TICTACTOEOVERLAN_INTERNALGAMESERVER_H
#define TICTACTOEOVERLAN_INTERNALGAMESERVER_H

#include <atomic>
#include <vector>
#include <winsock2.h>

#include "ClientContext.h"
#include "../common/NetworkProtocol.h"

#pragma comment(lib, "Ws2_32.lib")

class InternalGameServer {
    std::atomic<bool> keepRunning;
    SOCKET listenSocket;

    std::vector<ClientContext> clients;
    uint8_t nextPlayerId = 1;

    //Game State
    BoardData boardData;
    std::vector<Move> moves;
    // The clientContexts also hold player data and state

public:
    InternalGameServer():
    keepRunning(false),
    listenSocket(INVALID_SOCKET),
    boardData({{}, 3, 3, 0, 1}) {};

    std::atomic<long long> tick = 0;

    void start(int port);
    void stop();

private:
    void handleNewConnection();
    void handleClientData(ClientContext& client);
    void disconnectClient(size_t index);

    void processPacket(ClientContext& client, PacketType type, std::vector<char>& payload);

    template<typename T>
    static void sendPacket(SOCKET sock, PacketType type, const T &data);

    template<typename T>
    void broadcastPacket(const PacketType type, const T &data);
};


#endif //TICTACTOEOVERLAN_INTERNALGAMESERVER_H
