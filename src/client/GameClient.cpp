#include "GameClient.h"

#include <thread>

#include "SFML/Graphics/Text.hpp"

GameClient::GameClient() {
    clientState = ClientState::MENU;
    userInputIP = "127.0.0.1:27015";
    playerName = "Player1";

    //Later updated by the server
    // initialToken = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    initialToken = 2137696969;
    playerId = 1;
    pieceType = PieceType::EMPTY;

    //Debug stuff
    std::printf(ANSI_CYAN "[GameClient] Initial token %d\n", initialToken);
    //

    //Initialize the board with default settings
    boardData.boardSize = 3;
    boardData.winConditionLength = 3;
    boardData.turn = 0;
    boardData.currentPlayerId = 0;
    initializeGameBoard();

    window.create(sf::VideoMode({1000, 600}), "TicTacToe Ultimate Edition");
    window.setFramerateLimit(60);

    //TODO: adjust the file name here
    if (!font.openFromFile("../resources/JetBrainsMono-Regular.ttf")) {
        printf(ANSI_RED "[GameClient] Failed to load font\n" ANSI_RESET);
    }
}

void GameClient::run() {
    while (window.isOpen()) {
        handleInput();
        update();
        render();
    }
}

void GameClient::handleInput() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        switch (clientState) {
            case ClientState::MENU:
                handleMenuInput(event);
                break;

            case ClientState::GAME_ROOM:
                handleGameRoomInput(event);
                break;

            case ClientState::GAME:
                handleGameInput(event);
                break;
        }
    }
}

void GameClient::handleMenuInput(const std::optional<sf::Event> &event) {
    if (const auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Enter) {
            if (hosting == false) {
                hosting = true;
                startInternalServerThread();
            } else if (hosting == true) {
                hosting = false;
                stopInternalServerThread();
            }
        }
    }
}

void GameClient::handleGameRoomInput(const std::optional<sf::Event> &event) {
    //TODO:
}

void GameClient::handleGameInput(const std::optional<sf::Event> &event) {
    //TODO:
}

void GameClient::update() {
    PacketHeader header;
    std::vector<char> payload;

    while (networkManager.pollPacket(header, payload)) {
        //S2C packets: SERVER_HELLO, SETUP_ACK[x], NEW_PLAYER_JOIN, SETTINGS_UPDATE, GAME_START, BOARD_STATE_UPDATE, GAME_END
        switch (header.type) {
            case PacketType::SERVER_HELLO: {
                // if (clientState != ClientState::GAME_ROOM) {
                //     printf(ANSI_RED "Client isnt in the game room state, but we received a SERVER_HELLO packet\n" ANSI_RESET);
                // }
                setupPhase = SetupPhase::SETTING_UP;
                const auto *packet = reinterpret_cast<ServerHelloPacket *>(payload.data());
                playerId = packet->playerId;

                // Send the SETUP_REQ packet
                SetupReqPacket setupReqPacket;
                setupReqPacket.playerId = playerId;
                setupReqPacket.playerName = playerName;
                setupReqPacket.initialToken = initialToken;

                networkManager.sendPacket(PacketType::SETUP_REQ, &setupReqPacket);
            }

            case PacketType::SETUP_ACK: {
                // if (clientState != ClientState::GAME_ROOM) {
                //     printf(ANSI_RED "Client isnt in the game room state, but we received a SETUP_ACK packet\n" ANSI_RESET);
                // }

                const auto *packet = reinterpret_cast<SetupAckPacket *>(payload.data());
                authToken = packet->generatedAuthToken;
                playerName = packet->playerName;
                setupPhase = SetupPhase::CONNECTED;
            }

            case PacketType::NEW_PLAYER_JOIN: {
                // if (clientState != ClientState::GAME_ROOM) {
                //     printf(ANSI_RED "Client isn't in the game room state, but we received a NEW_PLAYER_JOIN packet\n" ANSI_RESET);
                // }

                const auto *packet = reinterpret_cast<NewPlayerJoinPacket *>(payload.data());
                Player newPlayer {};
                newPlayer.playerId = packet->newPlayerId;
                newPlayer.playerName = packet->newPlayerName;
                newPlayer.piece = packet->newPlayerPieceType;
                newPlayer.isMe = playerId == packet->newPlayerId;
                newPlayer.myTurn = false;
                players.push_back(newPlayer);
            }

            case PacketType::PLAYER_DISCONNECTED: {
                const auto *packet = reinterpret_cast<PlayerDisconnectedPacket *>(payload.data());
                //TODO: handle disconnect?? pause game?? Check if the disconnected player is me - this shouldnt happen
                std::erase_if(players, [packet](const Player &player) {
                    return player.playerId == packet->playerId;
                });
            }
        }
    }
}

void GameClient::render() {
    window.clear(sf::Color(BACKGROUND_COLOR));

    switch (clientState) {
        case ClientState::MENU:
            renderMenu();
        case ClientState::GAME_ROOM:
            renderGameRoom();
        case ClientState::GAME:
            renderGame();
    }

    window.display();
}

void GameClient::renderMenu() {
    //TODO:
    constexpr sf::Vector2f mainMenuPosition {16, 16}; //left corner
    sf::Text text(font);

    text.setString("Hello there!");
    text.setCharacterSize(20);
    text.setFillColor(sf::Color(TEXT_COLOR));
    text.setPosition({mainMenuPosition.x+20, mainMenuPosition.y+20});
    window.draw(text);

    std::string hostString = "Hosting: ";
    hostString += (hosting ? "True" : "False");
    text.setString(hostString);
    text.move({0, 30});
    window.draw(text);

}

void GameClient::renderGameRoom() {
    //TODO:
}

void GameClient::renderGame() {
    //TODO:
}

void GameClient::sendMove(uint8_t posX, uint8_t posY) {
    //TODO:
}

void GameClient::startInternalServerThread() {
    serverThread = std::thread([this]() {
            serverLogic.start(27015);
    });
}

void GameClient::stopInternalServerThread() {
    serverLogic.stop();

    if (serverThread.joinable()) {
        serverThread.join();
    }
}

void GameClient::initializeGameBoard() {
    std::vector<std::vector<BoardSquare> > grid;

    for (int i = 0; i < boardData.boardSize; ++i) {
        std::vector<BoardSquare> row;
        for (int j = 0; j < boardData.boardSize; ++j) {
            BoardSquare square{};
            square.piece = PieceType::EMPTY;
            row.push_back(square);
        }
        grid.push_back(row);
    }

    boardData.grid = grid;
}

GameClient::~GameClient() {
    stopInternalServerThread();
}



