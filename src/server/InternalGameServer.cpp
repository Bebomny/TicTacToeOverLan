#include "InternalGameServer.h"

#include <cstdio>
#include <thread>

#include "../common/NetworkProtocol.h"
#include "../common/Utils.h"

void InternalGameServer::start(const int port) {
    keepRunning = true;
    nextPlayerId = 1;

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
        // std::printf(ANSI_CYAN "[InternalServer] Hello from the server!\n" ANSI_RESET);

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
        // printf("SocketCount: %d\n", socketCount);


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

        //Calculate this based on how much time the processing took, set at 20 TPS initially -> 50ms per loop
        // std::this_thread::sleep_for(std::chrono_literals::operator ""ms(1000));
        tick++;
    }

    //Cleanup
    std::printf(ANSI_CYAN "[InternalServer] Shutting down...\n");
    closesocket(listenSocket);
    WSACleanup();
}

void InternalGameServer::handleNewConnection() { //TODO: reject new connections if a game is already in progress
    const SOCKET newSocket = accept(listenSocket, nullptr, nullptr);

    ClientContext newClient;
    newClient.setupPhase = ClientSetupPhase::NEW_CONNECTION;
    newClient.socket = newSocket;
    newClient.playerId = nextPlayerId++;

    ServerHelloPacket helloPacket;
    helloPacket.playerId = newClient.playerId;

    sendPacket<ServerHelloPacket>(newClient.socket, PacketType::SERVER_HELLO, helloPacket);
    newClient.setupPhase = ClientSetupPhase::HELLO_SENT;

    clients.push_back(std::move(newClient));
}

void InternalGameServer::handleClientData(ClientContext &client) {
    char buffer[DEFAULT_BUFFER_LEN];
    int bytesRead = recv(client.socket, buffer, sizeof(buffer), 0);

    printf(ANSI_YELLOW "[InternalServer] Got some new data!\n" ANSI_RESET);

    if (bytesRead <= 0) {
        //Error or 0 means disconnected
        std::printf(ANSI_RED "[InternalServer] Player %hhu disconnected.\n" ANSI_RESET, client.playerId);
        client.markedForDeletion = true;
        //TODO: broadcast player disconnect
    }

    client.receiveBuffer.insert(client.receiveBuffer.end(), buffer, buffer + bytesRead);

    //Packet parsing
    while (!client.markedForDeletion) {
        if (client.receiveBuffer.size() < sizeof(PacketHeader)) {
            break; //Not enough data, wait for next recv
        }

        const auto* pendingHeader = reinterpret_cast<PacketHeader *>(client.receiveBuffer.data());
        size_t totalPacketSize = sizeof(PacketHeader) + pendingHeader->payloadSize;

        if (client.receiveBuffer.size() < totalPacketSize) {
            break; //Still not enough data for the whole packet, wait for the next recv
        }

        // Here we have a valid packet
        std::vector<char> payload {};
        payload.clear();
        if (pendingHeader->payloadSize > 0) {
            payload.insert(
                payload.end(),
                client.receiveBuffer.begin() + sizeof(PacketHeader),
                client.receiveBuffer.begin() + totalPacketSize);
        }

        this->processPacket(client, pendingHeader->type, payload);

        client.receiveBuffer.erase(client.receiveBuffer.begin(), client.receiveBuffer.begin() + totalPacketSize);
    }
}

void InternalGameServer::processPacket(ClientContext &client, const PacketType type, std::vector<char>& payload) {
    //TODO: logic goes here
    //Packets: SETUP_REQ, SETTINGS_CHANGE_REQ, MOVE_REQ
    printf(ANSI_CYAN "[InternalServer] Received packet of type %hhd from client with ID: %hhu\n" ANSI_RESET, type, client.playerId);
    switch (type) {
        default: {
            printf(ANSI_RED "[InternalServer] Unknown packet received! Type: %hhd, Data: [%s]", type, &payload);
            break;
        }
        case PacketType::SETUP_REQ: {
            const auto *packet = reinterpret_cast<SetupReqPacket *>(payload.data());
            client.setupPhase = ClientSetupPhase::SETUP_REQ_RECV;

            //Respond with a generated token
            int clientAuthToken = packet->initialToken / 3; //TODO: change this, and generate them based on the current time and server token
            auto clientPieceType = static_cast<PieceType>(packet->playerId); //TODO: verify if the piece is available and stuff
            //TODO: validate the playername here
            printf(ANSI_CYAN "[InternalServer] Received SETUP_ACK with parameters [%hhu, %s, %d]\n" ANSI_RESET, packet->playerId, packet->playerName, packet->initialToken);

            SetupAckPacket setupAckPacket {};
            setupAckPacket.generatedAuthToken = clientAuthToken;
            setupAckPacket.playerId = packet->playerId;

            memset(setupAckPacket.playerName, 0, MAX_PLAYER_NAME_LENGTH);
            strncpy(setupAckPacket.playerName, packet->playerName, MAX_PLAYER_NAME_LENGTH-1);
            // setupAckPacket.playerName = packet->playerName;
            setupAckPacket.pieceType = clientPieceType;

            printf(ANSI_CYAN "[InternalServer] Sending SETUP_ACK packet to client with ID: %d\n" ANSI_RESET, packet->playerId);
            this->sendPacket(client.socket, PacketType::SETUP_ACK, setupAckPacket);

            //Add to playerlist - modify the client context (Or a separate active player list?)
            client.playerToken = clientAuthToken;
            client.pieceType = clientPieceType;
            client.playerName = packet->playerName;

            //broadcast new player joined packet
            NewPlayerJoinPacket newPlayerJoinPacket {};
            newPlayerJoinPacket.newPlayerId = client.playerId;
            newPlayerJoinPacket.newPlayerPieceType = clientPieceType;
            memset(newPlayerJoinPacket.newPlayerName, 0, MAX_PLAYER_NAME_LENGTH);
            strncpy(newPlayerJoinPacket.newPlayerName, client.playerName.c_str(), MAX_PLAYER_NAME_LENGTH-1);
            // newPlayerJoinPacket.newPlayerName = client.playerName;

            this->broadcastPacket(PacketType::NEW_PLAYER_JOIN, newPlayerJoinPacket);

            client.setupPhase = ClientSetupPhase::SET_UP;
            break;
        }
    }
}

void InternalGameServer::disconnectClient(size_t index) {
    //TODO:
}



template<typename T>
void InternalGameServer::broadcastPacket(const PacketType type, const T &data) {
    for (const auto &client: clients) {
        this->sendPacket(client.socket, type, data);
    }
}


template<typename T>
void InternalGameServer::sendPacket(const SOCKET sock, const PacketType type, const T &data) {
    //TODO: duplicate code from the client/NetworkManager merge them in the common area?
    std::vector<char> buffer;
    buffer.reserve(sizeof(PacketHeader) + sizeof(T));

    PacketHeader header {};
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
    nextPlayerId = 1;
}
