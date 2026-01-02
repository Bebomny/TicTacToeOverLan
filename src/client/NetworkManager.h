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

    int connectToServer(const std::string &address, const std::string &port);
    void disconnect();
    bool pollPacket(PacketHeader &outHeader, std::vector<char> &outPayload);

    template<typename T>
    void sendPacket(const PacketType type, const T &data) {
        if (conPhase != ConnectionPhase::ESTABLISHED) {
            printf(ANSI_RED "[SockClient] Attempting to send packet before a connection was made \n" ANSI_RESET);
            return;
        }

        std::vector<char> buffer;
        buffer.reserve(sizeof(PacketHeader) + sizeof(T));

        PacketHeader header;
        header.type = type;
        header.payloadSize = sizeof(T);

        const auto headerPtr = reinterpret_cast<const char *>(&header);
        buffer.insert(buffer.end(), headerPtr, headerPtr + sizeof(header));

        const auto dataPtr = reinterpret_cast<const char *>(&data);
        buffer.insert(buffer.end(), dataPtr, dataPtr + sizeof(T));

        int totalSent = 0;
        int bytesLeft = static_cast<int>(buffer.size());

        while (totalSent < buffer.size()) {
            const int sent = send(clientSocket, buffer.data() + totalSent, bytesLeft, 0);

            if (sent == -1) {
                // Error handling (connection lost?)
                printf(ANSI_RED "[SockClient] Error sending data!\n" ANSI_RESET);
                return;
            }

            totalSent += sent;
            bytesLeft -= sent;
        }
    }
};

#endif //TICTACTOEOVERLAN_NETWORKMANAGER_H
