#include "InternalGameServer.h"

#include <cstdio>
#include <ranges>
#include <thread>

#include "ServerUtils.h"
#include "WinValidator.h"
#include "../common/NetworkProtocol.h"
#include "../common/Utils.h"

void InternalGameServer::start(const int port) {
    keepRunning = true;
    nextPlayerId = 1;

    //GameState preparation
    //TODO: move this to a reset fuction
    boardData.boardSize = 3;
    boardData.winConditionLength = 3;
    boardData.round = 0;
    Utils::initializeGameBoard(boardData);
    availablePieces = {
        PieceType::HEXAGON,
        PieceType::OCTAGON,
        PieceType::SQUARE,
        PieceType::TRIANGLE,
        PieceType::CIRCLE,
        PieceType::CROSS
    };

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
        //Time measuring
        long long startTime = std::chrono::system_clock::now().time_since_epoch().count();

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

        // std::erase_if(
        //     clients,
        //     [](const auto& pair) { return pair.second.markedForDeletion; }
        // );
        std::erase_if(
            clients,
            [](const ClientContext& c) {return c.markedForDeletion;}
            );

        //Calculate this based on how much time the processing took, set at 20 TPS initially -> 50ms per loop
        // std::this_thread::sleep_for(std::chrono_literals::operator ""ms(1000));
        long long endTime = std::chrono::system_clock::now().time_since_epoch().count();
        long long timeTook = endTime - startTime;
        lastTickTime = timeTook;
        avgTickTime.add(timeTook);
        ++tick;
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
    newClient.playerWins = 0;

    ServerHelloPacket helloPacket;
    helloPacket.playerId = newClient.playerId;

    sendPacket<ServerHelloPacket>(newClient.socket, PacketType::SERVER_HELLO, helloPacket);
    newClient.setupPhase = ClientSetupPhase::HELLO_SENT;

    // clients.insert({newClient.playerId, newClient});
    clients.push_back(newClient);
}

void InternalGameServer::handleClientData(ClientContext &client) {
    char buffer[DEFAULT_BUFFER_LEN];
    int bytesRead = recv(client.socket, buffer, sizeof(buffer), 0);

    // printf(ANSI_YELLOW "[InternalServer] Got some new data!\n" ANSI_RESET);

    if (bytesRead <= 0) {
        //Error or 0 means disconnected
        std::printf(ANSI_RED "[InternalServer] Player with ID %hhu has disconnected.\n" ANSI_RESET, client.playerId);
        client.markedForDeletion = true;
        availablePieces.push_back(client.pieceType);

        closesocket(client.socket);
        client.socket = INVALID_SOCKET;

        PlayerDisconnectedPacket disconnectPacket {};
        disconnectPacket.playerId = client.playerId;
        this->broadcastPacket(PacketType::PLAYER_DISCONNECTED, disconnectPacket);

        // Reset the game here back to GAME_ROOM state
        GameEndPacket endPacket {};
        endPacket.reason = FinishReason::PLAYER_DISCONNECT;
        endPacket.playerId = client.playerId;
        endPacket.player = ServerUtils::clientContextToPlayer(client, 0);

        this->broadcastPacket(PacketType::GAME_END, endPacket);

        return;
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
            client.isHost = packet->isHost;
            if (client.isHost) hostingPlayerId = packet->playerId;

            //Respond with a generated token
            const int clientAuthToken = packet->initialToken / 3; //TODO: change this, and generate them based on the current time and server token
            const auto clientPieceType = this->getFirstAvailablePiece();
            //TODO: validate the playername here
            printf(ANSI_CYAN "[InternalServer] Received SETUP_ACK with parameters [%hhu, %s, %d]\n" ANSI_RESET, packet->playerId, packet->playerName, packet->initialToken);

            //Add to playerlist - modify the client context (Or a separate active player list?)
            client.playerToken = clientAuthToken;
            client.pieceType = clientPieceType;
            client.playerWins = 0;
            memset(client.playerName, 0, MAX_PLAYER_NAME_LENGTH);
            strncpy(client.playerName, packet->playerName, MAX_PLAYER_NAME_LENGTH-1);

            SetupAckPacket setupAckPacket {};
            setupAckPacket.generatedAuthToken = clientAuthToken;
            setupAckPacket.playerId = packet->playerId;
            //TODO: boardsize, winconditionlength and current players
            setupAckPacket.boardSize = boardData.boardSize;
            setupAckPacket.winConditionLength = boardData.winConditionLength;
            setupAckPacket.round = boardData.round;
            setupAckPacket.playerCount = 0;
            for (const auto &playerContext: clients) {
                if (setupAckPacket.playerCount >= MAX_PLAYERS) {
                    break;
                }
                setupAckPacket.players[setupAckPacket.playerCount] =
                    ServerUtils::clientContextToPlayer(playerContext, client.playerId);
                setupAckPacket.playerCount++;
            }

            memset(setupAckPacket.playerName, 0, MAX_PLAYER_NAME_LENGTH);
            strncpy(setupAckPacket.playerName, client.playerName, MAX_PLAYER_NAME_LENGTH-1);
            // setupAckPacket.playerName = packet->playerName;
            setupAckPacket.pieceType = clientPieceType;

            printf(ANSI_CYAN "[InternalServer] Sending SETUP_ACK packet to client with ID: %d\n" ANSI_RESET, packet->playerId);
            this->sendPacket(client.socket, PacketType::SETUP_ACK, setupAckPacket);

            //broadcast new player joined packet
            NewPlayerJoinPacket newPlayerJoinPacket {};
            newPlayerJoinPacket.newPlayerId = client.playerId;
            newPlayerJoinPacket.newPlayerPieceType = clientPieceType;
            newPlayerJoinPacket.isHost = client.isHost;
            memset(newPlayerJoinPacket.newPlayerName, 0, MAX_PLAYER_NAME_LENGTH);
            strncpy(newPlayerJoinPacket.newPlayerName, client.playerName, MAX_PLAYER_NAME_LENGTH-1);
            // newPlayerJoinPacket.newPlayerName = client.playerName;

            this->broadcastPacket(PacketType::NEW_PLAYER_JOIN, newPlayerJoinPacket);

            client.setupPhase = ClientSetupPhase::SET_UP;
            break;
        }

        case PacketType::SETTINGS_CHANGE_REQ: {
            const auto *packet = reinterpret_cast<SettingsChangeReqPacket *>(payload.data());
            printf(ANSI_CYAN "[InternalServer] Received SETTINGS_CHANGE_REQ packet from %hhu with params: [size: %hhu, winCon: %hhu]!\n" ANSI_RESET,
                packet->playerId, packet->newBoardSize, packet->newWinConditionLength);

            // TODO: Validate the player requesting the change, if they are the host

            bool updated = false;
            // 0 < BoardSize < MAX_BOARD_SIZE
            if (packet->newBoardSize > 0 && packet->newBoardSize <= MAX_BOARD_SIZE && boardData.boardSize != packet->newBoardSize) {
                printf(ANSI_GREEN "[InternalServer] BoardSize updated from %hhu to %hhu\n" ANSI_RESET, boardData.boardSize, packet->newBoardSize);
                boardData.boardSize = packet->newBoardSize;
                boardData.winConditionLength = std::min(boardData.winConditionLength, boardData.boardSize);
                updated = true;
            }

            // 0 < WinConditionLength < BoardSize
            if (packet->newWinConditionLength > 0 && packet->newWinConditionLength <= boardData.boardSize && boardData.winConditionLength != packet->newWinConditionLength) {
                printf(ANSI_GREEN "[InternalServer] WinConditionLength updated from %hhu to %hhu\n" ANSI_RESET, boardData.winConditionLength, packet->newWinConditionLength);
                boardData.winConditionLength = packet->newWinConditionLength;
                updated = true;
            }

            //Send update packet if necessary
            if (updated) {
                SettingsUpdatePacket settingsUpdatePacket {};
                settingsUpdatePacket.newBoardSize = boardData.boardSize;
                settingsUpdatePacket.newWinConditionLength = boardData.winConditionLength;

                printf(ANSI_CYAN "[InternalServer] Broadcasting new board settings!\n" ANSI_RESET);
                this->broadcastPacket(PacketType::SETTINGS_UPDATE, settingsUpdatePacket);
            }

            break;
        }

        case PacketType::GAME_START_REQ: {
            const auto *packet = reinterpret_cast<GameStartRequestPacket *>(payload.data());
            printf(ANSI_CYAN "[InternalServer] Got a%s game start request from player with id %hhu\n" ANSI_RESET,
                (packet->newGame ? " new" : ""), packet->requestingPlayerId);

            if (packet->requestingPlayerId != hostingPlayerId) {
                printf(ANSI_RED "[InternalServer] Somehow got a game start request from a client that isn't the host! "
                                "This shouldn't happen! [request: %hhu != host: %hhu]\n" ANSI_RESET,
                                packet->requestingPlayerId, hostingPlayerId);
                break;
            }

            Utils::initializeGameBoard(boardData);
            boardData.actingPlayerId = 1; //this->getNextActingPlayerId() //TODO: fix this make the first player in order start
            boardData.turn = 0;
            moves.clear();

            //TODO: [Idea] Make this reset via a button on the host instead?
            if (packet->newGame) {
                for (auto & ctx : clients) {
                    ctx.playerWins = 0;
                }
            }

            GameStartPacket gameStartPacket {};
            gameStartPacket.requestedByPlayerId = hostingPlayerId;
            gameStartPacket.finalBoardSize = boardData.boardSize;
            gameStartPacket.finalWinConditionLength = boardData.winConditionLength;
            gameStartPacket.round = boardData.round;
            gameStartPacket.turn = boardData.turn;
            gameStartPacket.startingPlayerId = boardData.actingPlayerId;
            gameStartPacket.playerCount = clients.size(); //To confirm we have synced the players on both sides
            Utils::serializeBoard(boardData, gameStartPacket.grid, TOTAL_BOARD_AREA);

            printf(ANSI_GREEN "[InternalServer] Sending out game start packets! [Starting playerID: %hhu]\n" ANSI_RESET,
                gameStartPacket.startingPlayerId);
            this->broadcastPacket(PacketType::GAME_START, gameStartPacket);

            break;
        }

        case PacketType::MOVE_REQ: {
            const auto *packet = reinterpret_cast<MoveRequestPacket *>(payload.data());
            printf(ANSI_CYAN "[InternalServer] Received a MOVE_REQ packet from player with ID: %hhu\n" ANSI_RESET, packet->playerId);
            if (packet->playerId != boardData.actingPlayerId) {
                printf(ANSI_RED "[InternalServer] Somehow got a move request from a player whose ID doesnt match the current acting players! [req: %hhu != currActing: %hhu]\n" ANSI_RESET,
                    packet->playerId, boardData.actingPlayerId);
                break;
            }

            if (packet->turn != boardData.turn) {
                printf(ANSI_RED "[InternalServer] Turn mismatch! Possible desync! Sending BoardStateUpdate to fix.\n" ANSI_RESET);
                //TODO: send boardstate update
                break;
            }

            //TODO: Check if the move is valid, check if x and y are within bounds
            if (boardData.getSquareAt(packet->x, packet->y).piece != PieceType::EMPTY) {
                printf(ANSI_YELLOW "[InternalServer] Player with id %hhu tried placing a piece on an already used square! [x:%hhu, y:%hhu]\n" ANSI_RESET,
                    packet->playerId, packet->x, packet->y);
                //TODO: send an alert to the client or check this on the client too
                break;
            }

            BoardSquare square {};
            square.playerId = packet->playerId;
            square.turnPlaced = boardData.turn;
            square.piece = packet->piece;
            boardData.setSquareAt(packet->x, packet->y, square);

            //For the move history
            Move move(packet->piece, packet->playerId, boardData.turn, packet->x, packet->y);
            moves.push_back(std::move(move));

            //Update the board state
            boardData.turn += 1;
            boardData.actingPlayerId = this->getNextActingPlayerId();

            // Broadcast the updated state
            BoardStateUpdatePacket boardUpdate {};
            Utils::serializeBoard(boardData, boardUpdate.grid, TOTAL_BOARD_AREA);
            boardUpdate.boardSize = boardData.boardSize;
            boardUpdate.winConditionLength = boardData.winConditionLength;
            boardUpdate.round = boardData.round;
            boardUpdate.turn = boardData.turn;
            boardUpdate.actingPlayerId = boardData.actingPlayerId;
            boardUpdate.lastMove = move;
            boardUpdate.playerCount = 0;
            for (auto &playerContext : clients) {
                if (boardUpdate.playerCount >= MAX_PLAYERS) {
                    break;
                }

                playerContext.myTurn = playerContext.playerId == boardData.actingPlayerId;

                boardUpdate.players[boardUpdate.playerCount] =
                    ServerUtils::clientContextToPlayer(playerContext, client.playerId);
                boardUpdate.playerCount++;
            }

            printf(ANSI_CYAN "[InternalServer] Broadcasting board state update packets!\n" ANSI_RESET);
            this->broadcastPacket(PacketType::BOARD_STATE_UPDATE, boardUpdate);


            bool gameFinished = WinValidator::checkWin(boardData, packet->x, packet->y);
            // bool gameFinished = false;
            if (gameFinished) {
                printf(ANSI_GREEN "[InternalServer] Player with ID %hhu won the round!\n" ANSI_RESET, packet->playerId);
                ClientContext *winningClient;
                for (auto &ctx : clients) {
                    if (ctx.playerId == packet->playerId) {
                        ctx.playerWins += 1;
                        winningClient = &ctx;
                        break;
                    }
                }
                boardData.round += 1;

                //Broadcast game finish
                GameEndPacket gameEndPacket {};
                gameEndPacket.reason = FinishReason::PLAYER_WIN;
                gameEndPacket.playerId = packet->playerId;
                gameEndPacket.player = ServerUtils::clientContextToPlayer(*winningClient, 0);

                this->broadcastPacket(PacketType::GAME_END, gameEndPacket);
            }
            break;
        }

        case PacketType::BACK_TO_GAME_ROOM: {
            const auto *packet = reinterpret_cast<BackToGameRoomPacket *>(payload.data());
            printf(ANSI_CYAN "[InternalServer] Got a BACK_TO_GAME_ROOM packet, relaying to all clients.\n" ANSI_RESET);
            // Relay the packet
            this->broadcastPacket(PacketType::BACK_TO_GAME_ROOM, packet);
        }
    }
}

void InternalGameServer::disconnectClient(size_t index) {
    //TODO: make the host be able to kick people from the game room
}



template<typename T>
void InternalGameServer::broadcastPacket(const PacketType type, const T &data) {
    for (const auto &client: clients) {
        if (client.markedForDeletion) continue;
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

PieceType InternalGameServer::getFirstAvailablePiece() {
    const PieceType piece = availablePieces.back();
    availablePieces.pop_back();
    return piece;
}


uint8_t InternalGameServer::getNextActingPlayerId() {
    // return clients[(boardData.round + boardData.turn) % clients.size()].playerId;
    return clients[(boardData.turn) % clients.size()].playerId;
}


void InternalGameServer::stop() {
    keepRunning = false;
    nextPlayerId = 1;
}

long long InternalGameServer::getTick() {
    return tick;
}

long long InternalGameServer::getLastTickTime() {
    return lastTickTime;
}

double InternalGameServer::getAvgTickTime() {
    return avgTickTime.average();
}

//TODO: check if there is a need to make these variables atomic, they may crash otherwise or trigger undefined behaviour
uint16_t InternalGameServer::getCurrentTurn() {
    return boardData.turn;
}

uint8_t InternalGameServer::getHostingPlayerId() {
    return hostingPlayerId;
}

uint8_t InternalGameServer::getNextPlayerId() {
    return nextPlayerId;
}

std::tuple<uint8_t, uint8_t> InternalGameServer::getBoardSettings() {
    return {boardData.boardSize, boardData.winConditionLength};
}

std::vector<PieceType> InternalGameServer::getAllAvailablePieces() {
    return availablePieces;
}

std::vector<Player> InternalGameServer::getPlayers() {
    std::vector<Player> players;
    players.reserve(clients.size());
    for (auto &client : clients) {
        players.push_back(ServerUtils::clientContextToPlayer(client, 0));
    }
    return players;
}

std::vector<Move> InternalGameServer::getMoves() {
    return moves;
}



