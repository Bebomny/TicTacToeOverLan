#include "GameClient.h"

#include <ranges>
#include <thread>

#include "SFML/Graphics/Text.hpp"
#include "ui/ButtonBuilder.h"

GameClient::GameClient() {
    clientState = ClientState::MENU;
    userInputIP = "localhost:27015";
    playerName = "Player";
    setupPhase = SetupPhase::DISCONNECTED;

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
    this->initializeGameBoard();
    playerCount = 0;
    players = {};

    window.create(sf::VideoMode({1500, 600}), "TicTacToe Ultimate Editionn");
    window.setFramerateLimit(60);

    //TODO: adjust the file name here
    if (!font.openFromFile("../resources/JetBrainsMono-Regular.ttf")) {
        printf(ANSI_RED "[GameClient] Failed to load font\n" ANSI_RESET);
    }
    ButtonBuilder::initFont(font);

    //TODO: move to separate Screen objects,
    this->initMenuWidgets();
    this->initGameRoomWidgets();
    this->initGameWidgets();

    printf(ANSI_GREEN "[GameClient] Setup finished!\n" ANSI_RESET);
}

void GameClient::initMenuWidgets() {
    // Host Button
    menuButtons.insert({
            "host",
            ButtonWidget::builder(
                "HOST",
                [this]() {
                    if (hosting == false) {
                        hosting = true;
                        this->startInternalServerThread();
                    } else if (hosting == true) {
                        hosting = false;
                        this->stopInternalServerThread();
                    }
                })
            .setPosition(mainMenuPosition.x, mainMenuPosition.y + 3 * defaultWidgetYOffset + 3)
            .build()
        }
    );

    // Connect button
    menuButtons.insert({
            "connect",
            ButtonWidget::builder(
                "Connect",
                [this]() {
                    this->connectAndSetup();
                })
            .setPosition(mainMenuPosition.x, mainMenuPosition.y + 2 * defaultWidgetYOffset + 1)
            .setSize(100, 24)
            .build()
        }
    );
}

void GameClient::initGameRoomWidgets() {
    // Start game button
    gameRoomButtons.insert({
        "start", ButtonWidget::builder(
            "Start",
            [this]() {
                this->startGame();
            })
        .setPosition(gameRoomPosition.x, gameRoomPosition.y + 5 * defaultWidgetYOffset)
        .setTextSize(26)
        .build()
    });

    // +- buttons for boardSize - validated on the server
    gameRoomButtons.insert({
        "boardsizeplus",
        ButtonWidget::builder(
            "+",
            [this]() {
                SettingsChangeReqPacket changeReq {};
                changeReq.playerId = playerId;
                changeReq.authToken = authToken;
                changeReq.newBoardSize = boardData.boardSize + 1;
                changeReq.newWinConditionLength = boardData.winConditionLength;

                printf(ANSI_CYAN "[GameClient] Sending settings change request packet with increased boardSize by 1\n" ANSI_RESET);
                this->networkManager.sendPacket(PacketType::SETTINGS_CHANGE_REQ, changeReq);
            })
        .setPosition(202, gameRoomPosition.y + 2 * defaultWidgetYOffset+1)
        .setSize(24, 24)
        .build()
    });

    gameRoomButtons.insert({
        "boardsizeminus",
        ButtonWidget::builder(
            "-",
            [this]() {
                SettingsChangeReqPacket changeReq {};
                changeReq.playerId = playerId;
                changeReq.authToken = authToken;
                changeReq.newBoardSize = boardData.boardSize - 1;
                changeReq.newWinConditionLength = boardData.winConditionLength;

                printf(ANSI_CYAN "[GameClient] Sending settings change request packet with decreased boardSize by 1\n" ANSI_RESET);
                this->networkManager.sendPacket(PacketType::SETTINGS_CHANGE_REQ, changeReq);
            })
        .setPosition(232, gameRoomPosition.y + 2 * defaultWidgetYOffset+1)
        .setSize(24, 24)
        .build()
    });

    //+- buttons for win condition length - validated on the server
    gameRoomButtons.insert({
        "winconditionplus",
        ButtonWidget::builder(
            "+",
            [this]() {
                SettingsChangeReqPacket changeReq {};
                changeReq.playerId = playerId;
                changeReq.authToken = authToken;
                changeReq.newBoardSize = boardData.boardSize;
                changeReq.newWinConditionLength = boardData.winConditionLength + 1;

                printf(ANSI_CYAN "[GameClient] Sending settings change request packet with increased win condition length by 1\n" ANSI_RESET);
                this->networkManager.sendPacket(PacketType::SETTINGS_CHANGE_REQ, changeReq);
            })
        .setPosition(284, gameRoomPosition.y + 3 * defaultWidgetYOffset+1)
        .setSize(24, 24)
        .build()
    });

    gameRoomButtons.insert({
        "winconditionminus",
        ButtonWidget::builder(
            "-",
            [this]() {
                SettingsChangeReqPacket changeReq {};
                changeReq.playerId = playerId;
                changeReq.authToken = authToken;
                changeReq.newBoardSize = boardData.boardSize;
                changeReq.newWinConditionLength = boardData.winConditionLength - 1;

                printf(ANSI_CYAN "[GameClient] Sending settings change request packet with decreased win condition length by 1\n" ANSI_RESET);
                this->networkManager.sendPacket(PacketType::SETTINGS_CHANGE_REQ, changeReq);
            })
        .setPosition(314, gameRoomPosition.y + 3 * defaultWidgetYOffset+1)
        .setSize(24, 24)
        .build()
    });
}

void GameClient::initGameWidgets() {
    //TODO:
}

void GameClient::run() {
    while (window.isOpen()) {
        this->handleInput();
        this->update();
        this->render();
    }
}

void GameClient::handleInput() {
    const sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    //Update buttons
    for (const auto &btn: menuButtons | std::views::values) {
        btn->update(mousePos);
    }

    for (const auto & [id, btn]: gameRoomButtons) {
        if (!hosting && (id == "Start" || id == "boardsizeplus" || id == "boardsizeminus" || id == "winconditionplus" || id == "winconditionminus")) {
            continue;
        }
        btn->update(mousePos);
    }

    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        //Absolute key events, handled in any client state
        if (const auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::F3) {
                debugEnabled = !debugEnabled;
            }
        }

        switch (clientState) {
            case ClientState::MENU:
                this->handleMenuInput(event, mousePos);
                break;

            case ClientState::GAME_ROOM:
                this->handleGameRoomInput(event, mousePos);
                break;

            case ClientState::GAME:
                this->handleGameInput(event, mousePos);
                break;
        }
    }
}

void GameClient::handleMenuInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) {
    for (const auto &btn: menuButtons | std::views::values) {
        btn->handleEvent(event, mousePos);
    }

    //Remove this later //Debug code
    if (const auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Enter) {
            if (hosting == false) {
                hosting = true;
                this->startInternalServerThread();
            } else if (hosting == true) {
                hosting = false;
                this->stopInternalServerThread();
            }
        }

        if (keyEvent->code == sf::Keyboard::Key::RShift) {
            this->connectAndSetup();
        }
    }
    //
}

void GameClient::handleGameRoomInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) {
    //TODO: add disconnect
    for (const auto &[id, btn]: gameRoomButtons) {
        if (!hosting && (id == "Start" || id == "boardsizeplus" || id == "boardsizeminus" || id == "winconditionplus" || id == "winconditionminus")) {
            continue;
        }
        btn->handleEvent(event, mousePos);
    }
}

void GameClient::handleGameInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) {
    //TODO:
}

void GameClient::update() {
    PacketHeader header{};
    std::vector<char> payload;

    while (networkManager.pollPacket(header, payload)) {
        //S2C packets: SERVER_HELLO, SETUP_ACK[x], NEW_PLAYER_JOIN, SETTINGS_UPDATE, GAME_START, BOARD_STATE_UPDATE, GAME_END
        switch (header.type) {
            case PacketType::SERVER_HELLO: {
                if (clientState != ClientState::GAME_ROOM) {
                    printf(ANSI_RED "Client isnt in the game room state, but we received a SERVER_HELLO packet\n" ANSI_RESET);
                }
                printf(ANSI_CYAN "[GameClient] Got Server Hello packet!\n" ANSI_RESET);
                setupPhase = SetupPhase::SETTING_UP;
                const auto *packet = reinterpret_cast<ServerHelloPacket *>(payload.data());
                playerId = packet->playerId;
                printf(ANSI_CYAN "[GameClient] Assigned player ID: %hhu\n" ANSI_RESET, packet->playerId);

                // Send the SETUP_REQ packet
                SetupReqPacket setupReqPacket{};
                setupReqPacket.playerId = playerId;
                memset(setupReqPacket.playerName, 0, MAX_PLAYER_NAME_LENGTH);
                strncpy(setupReqPacket.playerName, playerName.c_str(), MAX_PLAYER_NAME_LENGTH - 1);
                setupReqPacket.initialToken = initialToken;

                printf(
                    ANSI_CYAN "[GameClient] Sending SETUP_REQ packet with [pid:%hhu] [pName:%s] [initialToken:%d]\n"
                    ANSI_RESET, playerId, playerName.c_str(), initialToken);
                networkManager.sendPacket<SetupReqPacket>(PacketType::SETUP_REQ, setupReqPacket);
                printf(ANSI_CYAN "[GameClient] SETUP_REQ sent!\n");
                break;
            }

            case PacketType::SETUP_ACK: {
                if (clientState != ClientState::GAME_ROOM) {
                    printf(ANSI_RED "Client isn't in the game room state, but we received a SETUP_ACK packet\n" ANSI_RESET);
                }
                printf(ANSI_CYAN "[GameClient] Got SETUP_ACK packet!\n" ANSI_RESET);
                //TODO: check if the playerId is the same as ours
                const auto *packet = reinterpret_cast<SetupAckPacket *>(payload.data());
                authToken = packet->generatedAuthToken;
                pieceType = packet->pieceType;
                //This technically isn't necessary, but I should at least check if the server is happy with the chosen name
                playerName = packet->playerName;

                boardData.boardSize = packet->boardSize;
                boardData.winConditionLength = packet->winConditionLength;

                playerCount = packet->playerCount;
                players = {};
                //TODO: Fix sometimes the playerName comes as random gibberish, no idea why. Probably something in the server packet formation;
                for (const auto player: packet->players) {
                    if (player.playerId == NULL) {
                        continue;
                    }
                    printf("[%hhu, %s, %hhd, %hhd, %hhd]\n", player.playerId, player.playerName, player.piece,
                           player.myTurn, player.isMe);
                    players.push_back(player);
                }

                setupPhase = SetupPhase::CONNECTED;
                printf(ANSI_CYAN "[GameClient] Generated AuthToken: %d Other: [name:%s, id:%hhd]\n" ANSI_RESET,
                       authToken, playerName.c_str(), pieceType);
                break;
            }

            case PacketType::NEW_PLAYER_JOIN: {
                if (clientState != ClientState::GAME_ROOM) {
                    printf(ANSI_RED "Client isn't in the game room state, but we received a NEW_PLAYER_JOIN packet\n" ANSI_RESET);
                }

                //TODO: Check for duplicates and check if its me
                printf(ANSI_CYAN "[GameClient] Got NEW_PLAYER_JOIN packet!\n" ANSI_RESET);
                const auto *packet = reinterpret_cast<NewPlayerJoinPacket *>(payload.data());

                Player newPlayer{};
                newPlayer.playerId = packet->newPlayerId;
                // newPlayer.playerName = packet->newPlayerName;
                memset(newPlayer.playerName, 0, MAX_PLAYER_NAME_LENGTH);
                strncpy(newPlayer.playerName, packet->newPlayerName, MAX_PLAYER_NAME_LENGTH - 1);
                newPlayer.piece = packet->newPlayerPieceType;
                newPlayer.isMe = playerId == packet->newPlayerId;
                newPlayer.myTurn = false;
                // If a player with this id already exists overwrite it
                std::erase_if(
                    players, [packet](const Player &player) { return packet->newPlayerId == player.playerId; });
                players.push_back(newPlayer);
                break;
            }

            case PacketType::SETTINGS_UPDATE: {
                const auto *packet = reinterpret_cast<SettingsUpdatePacket *>(payload.data());
                printf(ANSI_CYAN "[GameClient] Got SETTINGS_UPDATE packet!\n" ANSI_RESET);
                printf(ANSI_GREEN "[GameClient] New Board Size: %hhu, New Win Condition Length: %hhu\n" ANSI_RESET,
                    packet->newBoardSize, packet->newWinConditionLength);
                boardData.boardSize = packet->newBoardSize;
                boardData.winConditionLength = packet->newWinConditionLength;
                break;
            }

            case PacketType::PLAYER_DISCONNECTED: {
                const auto *packet = reinterpret_cast<PlayerDisconnectedPacket *>(payload.data());
                //TODO: handle disconnect?? pause game?? Check if the disconnected player is me - this shouldnt happen
                std::erase_if(players, [packet](const Player &player) {
                    return player.playerId == packet->playerId;
                });
                break;
            }
        }
    }
}

void GameClient::render() {
    window.clear(sf::Color(BACKGROUND_COLOR));

    if (debugEnabled) {
        this->renderDebugMenu();
    }

    switch (clientState) {
        case ClientState::MENU:
            this->renderMenu();
            break;
        case ClientState::GAME_ROOM:
            this->renderGameRoom();
            break;
        case ClientState::GAME:
            this->renderGame();
            break;
    }
    
    window.display();
}

void GameClient::renderMenu() {
    //TODO:
    sf::Text text(font);

    text.setString((hosting ? "You are the host!" : "Hello there!"));
    text.setCharacterSize(defaultTextSize);
    text.setFillColor(sf::Color(TEXT_COLOR));
    text.setPosition({mainMenuPosition.x, mainMenuPosition.y});
    window.draw(text);

    text.setString("Player name: " + playerName);
    text.move({0, defaultWidgetYOffset});
    window.draw(text);

    text.setString("Server Address: " + userInputIP);
    text.move({110, defaultWidgetYOffset});
    window.draw(text);

    // Render buttons
    for (const auto &btn: menuButtons | std::views::values) {
        btn->render(window);
    }
}

void GameClient::renderGameRoom() {
    //TODO:
    sf::Text text(font);

    // Player name
    text.setString("Player name: " + playerName);
    text.setCharacterSize(defaultTextSize);
    text.setFillColor(sf::Color(TEXT_COLOR));
    text.setPosition({gameRoomPosition.x, gameRoomPosition.y});
    window.draw(text);

    // Am I the Host
    text.setString((hosting ? "You are the host!" : "Someone else is hosting!"));
    text.move({0, defaultWidgetYOffset});
    window.draw(text);

    // Board Size
    text.setString("Board Size: " + std::to_string(boardData.boardSize));
    text.move({0, defaultWidgetYOffset});
    window.draw(text);

    // Win Condition Length
    text.setString("Required in a row: " + std::to_string(boardData.winConditionLength));
    text.move({0, defaultWidgetYOffset});
    window.draw(text);

    // space
    text.move({0, defaultWidgetYOffset});

    // Start button for the host - initialized in the button widgets
    text.move({0, defaultWidgetYOffset});

    // space
    text.move({0, defaultWidgetYOffset});

    // Players
    text.setString("Players[" + std::to_string(players.size()) + ", " + std::to_string(playerCount) + "]:");
    text.move({0, defaultWidgetYOffset});
    window.draw(text);

    for (const auto &player: players) {
        std::string pNameString = player.playerName;
        std::string piece = Utils::pieceTypeToString(player.piece);
        text.setString("Name: " + pNameString
                       + "  ID: " + std::to_string(player.playerId)
                       + "  Piece: " + piece);
        text.move({0, defaultWidgetYOffset});
        window.draw(text);
    }

    // Render buttons
    for (const auto &[id, btn] : gameRoomButtons) {
        if (!hosting && (id == "Start" || id == "boardsizeplus" || id == "boardsizeminus" || id == "winconditionplus" || id == "winconditionminus")) {
            continue;
        }
        btn->render(window);
    }
}

void GameClient::renderGame() {
    //TODO:
}

void GameClient::renderDebugMenu() {
    const sf::Vector2u debugMenuPosition{window.getSize().x - 500, 16}; //left corner
    constexpr int textSize = 16;
    constexpr int textYOffset{textSize + textSize / 2};

    sf::Text text(font);
    text.setCharacterSize(textSize);
    text.setFillColor(sf::Color(TEXT_COLOR));

    std::string hostString = "Hosting: ";
    hostString += (hosting ? "True" : "False");
    text.setString(hostString);
    text.setPosition({static_cast<float>(debugMenuPosition.x + 20), static_cast<float>(debugMenuPosition.y + 20)});
    window.draw(text);

    text.setString("Server IP: " + serverAddress + ":" + serverPort);
    text.move({0, textYOffset});
    window.draw(text);

    text.setString("PlayerID: " + std::to_string(playerId));
    text.move({0, textYOffset});
    window.draw(text);

    text.setString("PlayerName: " + playerName);
    text.move({0, textYOffset});
    window.draw(text);

    text.setString("AuthToken: " + std::to_string(authToken));
    text.move({0, textYOffset});
    window.draw(text);

    std::string pieceTypeText = "PieceType: ";
    switch (pieceType) {
        case PieceType::EMPTY:
            pieceTypeText += "[0] Empty";
            break;
        case PieceType::CROSS:
            pieceTypeText += "[1] Cross";
            break;
        case PieceType::CIRCLE:
            pieceTypeText += "[2] Circle";
            break;
        case PieceType::TRIANGLE:
            pieceTypeText += "[3] Triangle";
            break;
        case PieceType::SQUARE:
            pieceTypeText += "[4] Square";
            break;
        case PieceType::DIAMOND:
            pieceTypeText += "[5] Diamond";
            break;
        case PieceType::OCTAGON:
            pieceTypeText += "[6] Octagon";
            break;
    }
    text.setString(pieceTypeText);
    text.move({0, textYOffset});
    window.draw(text);

    std::string connectionPhase = "Connection Phase: ";
    switch (networkManager.conPhase) {
        case ConnectionPhase::DISCONNECTED:
            connectionPhase += "[0] Disconnected";
            break;
        case ConnectionPhase::ESTABLISHING:
            connectionPhase += "[1] Establishing";
            break;
        case ConnectionPhase::ESTABLISHED:
            connectionPhase += "[2] Established";
            break;
    }
    text.setString(connectionPhase);
    text.move({0, textYOffset});
    window.draw(text);

    //TODO: Make this prettier -- why are enums like this in cpp???
    std::string setupPhaseString = "Setup Phase: ";
    switch (setupPhase) {
        case SetupPhase::DISCONNECTED:
            setupPhaseString += "[0] Disconnected";
            break;
        case SetupPhase::CONN_ESTABLISHED:
            setupPhaseString += "[1] Connection Established";
            break;
        case SetupPhase::SETTING_UP:
            setupPhaseString += "[2] Setting Up";
            break;
        case SetupPhase::CONNECTED:
            setupPhaseString += "[3] Connected";
            break;
    }
    text.setString(setupPhaseString);
    text.move({0, textYOffset});
    window.draw(text);

    std::string clientStateString = "Client State: ";
    switch (clientState) {
        case ClientState::MENU:
            clientStateString += "[0] Menu";
            break;
        case ClientState::GAME_ROOM:
            clientStateString += "[1] Game Room";
            break;
        case ClientState::GAME:
            clientStateString += "[2] Game";
            break;
    }
    text.setString(clientStateString);
    text.move({0, textYOffset});
    window.draw(text);

    std::string gamePhaseString = "Game Phase: ";
    switch (gamePhase) {
        case GamePhase::UNKNOWN:
            gamePhaseString += "[0] Unknown ";
            break;
        case GamePhase::WAITING_ROOM:
            gamePhaseString += "[1] Waiting Room";
            break;
        case GamePhase::MY_TURN:
            gamePhaseString += "[2] My Turn";
            break;
        case GamePhase::NOT_MY_TURN:
            gamePhaseString += "[3] Not My Turn";
            break;
    }
    text.setString(gamePhaseString);
    text.move({0, textYOffset});
    window.draw(text);

    std::string playerString = std::format("Players[{}]: [", players.size());
    for (auto player: players) {
        std::stringstream ss;
        ss << "{"
                << static_cast<int>(player.playerId) << ", "
                << player.playerName << ", "
                << static_cast<int>(player.piece)
                << "}";

        playerString += ss.str();
    }
    playerString += "]";
    text.setString(playerString);
    text.move({0, textYOffset});
    window.draw(text);

    //Internal Server stuff
    if (hosting) {
        text.setString("Internal Game Server Debug");
        text.move({0, textYOffset * 2});
        window.draw(text);

        text.setString("Current Tick: " + std::to_string(serverLogic.tick));
        text.move({0, textYOffset});
        window.draw(text);
    }
}

void GameClient::connectAndSetup() {
    //TODO: Validate user input here

    std::string serverIP = userInputIP;
    serverAddress = strtok(serverIP.data(), ":");
    serverPort = strtok(nullptr, ":");

    printf(ANSI_CYAN "[GameClient] Connecting to a server at %s... [%s, %s]\n" ANSI_RESET,
           userInputIP.c_str(), serverAddress.c_str(), serverPort.c_str());

    int conResult = networkManager.connectToServer(serverAddress, serverPort);
    if (conResult == 0 || conResult == 1) {
        clientState = ClientState::GAME_ROOM;
    } else {
        clientState = ClientState::MENU;
        printf(ANSI_RED "[GameClient] Failed to connect at %s...\n" ANSI_RESET, userInputIP.c_str());
    }
}

void GameClient::startGame() {
    //TODO:
}


void GameClient::sendMove(uint8_t posX, uint8_t posY) {
    //TODO:
}

void GameClient::startInternalServerThread() {
    printf(ANSI_CYAN "[GameClient] Internal Server is starting...\n" ANSI_RESET);
    serverThread = std::thread([this]() {
        serverLogic.start(27015);
    });
}

void GameClient::stopInternalServerThread() {
    printf(ANSI_YELLOW "[GameClient] Internal Server is stopping...\n" ANSI_RESET);
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
    if (hosting) {
        stopInternalServerThread();
    }
}



