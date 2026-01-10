#ifndef TICTACTOEOVERLAN_CLIENTCONTEXT_H
#define TICTACTOEOVERLAN_CLIENTCONTEXT_H

#include <cstdint>
#include <vector>
#include <winsock2.h>

#include "../common/GameDefinitions.h"
#include "../common/NetworkProtocol.h"

#pragma comment(lib, "Ws2_32.lib")

/**
 * @brief Tracks the handshake progress of a client on the Server side.
 */
enum class ClientSetupPhase {
    NEW_CONNECTION,
    HELLO_SENT,
    SETUP_REQ_RECV,
    SET_UP
};

/**
 * @brief Represents a connected client within the InternalGameServer.
 * <br> Holds the raw socket, buffering for network streams, and player-specific metadata.
 * <br> Unlike the `Player` struct, this struct manages the connection lifecycle and server-specific state.
 */
struct ClientContext {
    SOCKET socket;
    uint8_t playerId;
    mutable int32_t playerToken;
    mutable PieceType pieceType;
    mutable char playerName[MAX_PLAYER_NAME_LENGTH];
    mutable int32_t playerWins;
    mutable bool isHost;
    mutable bool myTurn;

    mutable ClientSetupPhase setupPhase;

    mutable std::vector<char> receiveBuffer {};

    mutable bool markedForDeletion = false;
};


#endif //TICTACTOEOVERLAN_CLIENTCONTEXT_H