#ifndef TICTACTOEOVERLAN_NETWORKMANAGER_H
#define TICTACTOEOVERLAN_NETWORKMANAGER_H
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../common/NetworkProtocol.h"
#include "../common/Utils.h"

#pragma comment(lib, "Ws2_32.lib")

enum class ConnectionPhase : uint8_t {
    DISCONNECTED,
    ESTABLISHING,
    ESTABLISHED
};

class NetworkManager {
public:
    ConnectionPhase conPhase = ConnectionPhase::DISCONNECTED;
    int startResult = -1;
    WSADATA wsadata = {};
    SOCKET clientSocket;
    std::vector<char> receiveBuffer;

    bool connectToServer(const std::string &address, const std::string &port);
    template <typename T> void sendPacket(PacketType type, const T& data);
    bool pollPacket(PacketHeader& outHeader, std::vector<char>& outPayload);
};

#endif //TICTACTOEOVERLAN_NETWORKMANAGER_H