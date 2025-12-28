#include "InternalGameServer.h"

#include <cstdio>
#include <thread>

#include "../common/NetworkProtocol.h"
#include "../common/Utils.h"

void InternalGameServer::start(int port) {
    keepRunning = true;

    //Socket and network setup
    WSADATA wsadata;
    WSAStartup(REQ_SOCK_VERSION, &wsadata);

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    bind(listenSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr));
    listen(listenSocket, SOMAXCONN);

    std::printf(ANSI_GREEN "[InternalServer] Listening on port %d...\n" ANSI_RESET, port);

    while (keepRunning) {
        //packets and logic
        std::printf(ANSI_CYAN "[InternalServer] Hello from the server!\n" ANSI_RESET);

        fd_set readSet;
        FD_ZERO(&readSet);

        FD_SET(listenSocket, &readSet);

        for (const auto &client: clients) {
            FD_SET(client.socket, &readSet);
        }

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        const int socketCount = select(0, &readSet, nullptr, nullptr, &timeout);

        if (socketCount > 0) {
            if (FD_ISSET(listenSocket, &readSet)) {
                this->handleNewConnection();
            }

            for (auto &client: clients) {
                if (FD_ISSET(client.socket, &readSet)) {
                    this->handleClientData(client);
                }
            }
        }

        std::erase_if(
            clients,
            [](const ClientContext &c) { return c.markedForDeletion; }
        );

        std::this_thread::sleep_for(std::chrono_literals::operator ""ms(1000));
    }

    //Cleanup
    std::printf(ANSI_CYAN "[InternalServer] Shutting down...\n");
    closesocket(listenSocket);
    WSACleanup();
}

void InternalGameServer::handleNewConnection() {
    const SOCKET newSocket = accept(listenSocket, nullptr, nullptr);

    ClientContext newClient;
    newClient.setupPhase = ClientSetupPhase::NEW_CONNECTION;
    newClient.socket = newSocket;
    newClient.playerId = nextPlayerId++;

    ServerHelloPacket helloPacket;
    helloPacket.playerId = newClient.playerId;

    sendPacket(newClient.socket, PacketType::SERVER_HELLO, &helloPacket);
    newClient.setupPhase = ClientSetupPhase::HELLO_SENT;
}

void InternalGameServer::handleClientData(const ClientContext &client) {
    char buffer[DEFAULT_BUFFER_LEN];
    int bytesRead = recv(client.socket, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0) {
        //Error or 0 means disconnected
        std::printf(ANSI_RED "[InternalServer] Player %hhu disconnected.\n" ANSI_RESET, client.playerId);
        client.markedForDeletion = true;
        //TODO: broadcast player disconnect
    }

    //TODO: logic goes here
}

template<typename T>
void InternalGameServer::broadcastPacket(const PacketType type, const T &data) {
    for (const auto &client: clients) {
        this->sendPacket(client, type, data);
    }
}


template<typename T>
void InternalGameServer::sendPacket(const SOCKET sock, const PacketType type, const T &data) {
    //TODO: duplicate code from the client/NetworkManager merge them in the common area?
    std::vector<char> buffer;
    buffer.reserve(sizeof(PacketHeader) + sizeof(T));

    PacketHeader header;
    header.type = type;
    header.payloadSize = sizeof(T);

    const auto headerPtr = reinterpret_cast<const char*>(&header);
    buffer.insert(buffer.end(), headerPtr, headerPtr + sizeof(header));

    const auto dataPtr = reinterpret_cast<const char*>(&data);
    buffer.insert(buffer.end(), dataPtr, dataPtr + sizeof(T));

    int totalSent = 0;
    int bytesLeft = static_cast<int>(buffer.size());

    while (totalSent < buffer.size()) {
        const int sent = send(sock, buffer.data() + totalSent, bytesLeft, 0);

        if (sent == -1) {
            // Error handling (connection lost?)
            printf(ANSI_RED "[InternalServer] Error sending data!\n" ANSI_RESET);
            return;
        }

        totalSent += sent;
        bytesLeft -= sent;
    }
}

void InternalGameServer::stop() {
    keepRunning = false;
}
