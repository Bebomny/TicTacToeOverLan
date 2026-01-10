#ifndef TICTACTOEOVERLAN_NETWORKMANAGER_H
#define TICTACTOEOVERLAN_NETWORKMANAGER_H
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../common/NetworkProtocol.h"
#include "../common/Utils.h"

#pragma comment(lib, "Ws2_32.lib")

/**
 * @brief Represents the state of the raw socket connection.
 */
enum class ConnectionPhase : uint8_t {
    DISCONNECTED,
    ESTABLISHING,
    ESTABLISHED
};

/**
 * @brief Manages low-level TCP network communication.
 * <br> Wraps the Winsock API to provide a cleaner interface
 * for connecting, disconnecting, and exchanging structured packets.
 * <br> Handles packet framing (Header + Payload) and ensures data integrity during sends.
 */
class NetworkManager {
public:
    ConnectionPhase conPhase = ConnectionPhase::DISCONNECTED;
    int startResult = -1;
    WSADATA wsadata = {};
    SOCKET clientSocket;
    std::vector<char> receiveBuffer;

    /**
     * @brief Attempts to establish a TCP connection to a server.
     * <br> Initializes Winsock, creates a socket, and attempts to connect to the specified end point.
     *
     * @param address The IP address (IPv4) or hostname of the server.
     * @param port The port number as a string.
     * @return 0 on success, non-zero error code on failure.
     */
    int connectToServer(const std::string &address, const std::string &port);

    /**
     * @brief Closes the socket and cleans up Winsock resources.
     */
    void disconnect();

    /**
     * @brief Checks the socket for incoming data and attempts to extract a single complete packet.
     * <br> This function handles TCP stream fragmentation. If enough data has arrived to form
     * a full packet (Header + defined Payload size), it extracts it.
     *
     * * @param outHeader Output parameter to store the parsed packet header.
     * @param outPayload Output parameter to store the raw byte payload.
     * @return True if a complete packet was successfully retrieved, False if there is insufficient data yet.
     */
    bool pollPacket(PacketHeader &outHeader, std::vector<char> &outPayload);

    /**
     * @brief Serializes and sends a structured packet to the server.
     * <br> Encapsulates the payload with a standard `PacketHeader` containing the type and size.
     * <br> Includes a loop to handle partial sends, ensuring the entire buffer is transmitted.
     *
     * @tparam T The type of the data structure being sent.
     * @param type The PacketType enum identifier.
     * @param data The actual data struct to send.
     */
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
