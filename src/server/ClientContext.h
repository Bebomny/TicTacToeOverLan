#ifndef TICTACTOEOVERLAN_CLIENTCONTEXT_H
#define TICTACTOEOVERLAN_CLIENTCONTEXT_H

#include <cstdint>
#include <string>
#include <vector>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

enum class ClientSetupPhase {
    NEW_CONNECTION,
    HELLO_SENT,
    SETUP_REQ_RECV,
    SET_UP
};

struct ClientContext {
    SOCKET socket;
    uint8_t playerId;
    int32_t playerToken;
    std::string playerName;
    ClientSetupPhase setupPhase;

    std::vector<char> receiveBuffer;

public:
    mutable bool markedForDeletion = false;
};


#endif //TICTACTOEOVERLAN_CLIENTCONTEXT_H