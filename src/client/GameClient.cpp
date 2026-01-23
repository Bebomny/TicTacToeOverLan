#include "GameClient.h"

#include <cmath>
#include <ranges>
#include <thread>

#include "../common/resources/JetBrainsMonoRegularFont.h"
#include "../common/resources/WindowIcon.h"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Text.hpp"
#include "ui/BoardRenderer.h"
#include "ui/DrawUtils.h"
#include "ui/TextFieldWidget.h"

GameClient::GameClient() {
    clientState = ClientState::MENU;
    userInputIP = "localhost:27015";
    playerName = "Player";
    setupPhase = SetupPhase::DISCONNECTED;
    finishReason = FinishReason::NONE;

    //Later updated by the server
    initialToken = std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1000000000;
    authToken = -1;
    playerId = -1;
    pieceType = PieceType::EMPTY;

    //Debug stuff
    std::printf(ANSI_CYAN "[GameClient] Initial token %d\n", initialToken);

    //Initialize the board with default settings
    boardData.boardSize = 3;
    boardData.winConditionLength = 3;
    boardData.round = 1;
    boardData.turn = 1;
    boardData.actingPlayerId = 0;
    Utils::initializeGameBoard(boardData);
    playerCount = 0;
    players = {};
    lastMove = {};
    gameEndPlayer = {};

    window.create(sf::VideoMode({1500, 600}), "TicTacToe Ultimate Editionn", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    clock.start();

    sf::Image icon;
    if (!icon.loadFromMemory(TicTacToeOverLan_ICON_DATA, sizeof(TicTacToeOverLan_ICON_DATA))) {
        printf(ANSI_RED "[GameClient] Failed to load the embedded icon\n" ANSI_RESET);
        return;
    }
    window.setIcon(icon);

    if (!font.openFromMemory(JetBrainsMono_Regular, JetBrainsMono_Regular_Len)) {
        printf(ANSI_RED "[GameClient] Failed to load the embedded font\n" ANSI_RESET);
        return;
    }

    ButtonBuilder::initFont(font);
    TextFieldBuilder::initFont(font);

    this->initWidgets();

    printf(ANSI_GREEN "[GameClient] Setup finished!\n" ANSI_RESET);
}

void GameClient::initWidgets() {
    //// Menu Widgets ////
    // Player name Text Field
    widgets.insert({
        "player_name_input",
        TextFieldWidget::builder(
            playerName,
            [this](const std::string &s) { this->playerName = s; })
        .setDisplayCondition([this]() { return this->clientState == ClientState::MENU; })
        .setPosition(MAIN_MENU_POSITION.x + 145, MAIN_MENU_POSITION.y + DEFAULT_WIDGET_Y_OFFSET - 1)
        .setMaxChars(20)
        .build()
    });

    // Server IP Text Field
    widgets.insert({
        "server_ip_input",
        TextFieldWidget::builder(
            userInputIP,
            [this](const std::string &s) { this->userInputIP = s; })
        .setDisplayCondition([this]() { return this->clientState == ClientState::MENU; })
        .setPosition(MAIN_MENU_POSITION.x + 170, MAIN_MENU_POSITION.y + (DEFAULT_WIDGET_Y_OFFSET * 2) - 1)
        .setMaxChars(20)
        .build()
    });

    // Host button
    widgets.insert({
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
            .setPosition(MAIN_MENU_POSITION.x, MAIN_MENU_POSITION.y + 4 * DEFAULT_WIDGET_Y_OFFSET + 3)
            .setDisplayCondition([this]() { return this->clientState == ClientState::MENU; })
            .build()
        }
    );

    // Connect button
    widgets.insert({
            "connect",
            ButtonWidget::builder(
                "Connect",
                [this]() {
                    this->connectAndSetup();
                })
            .setPosition(MAIN_MENU_POSITION.x, MAIN_MENU_POSITION.y + 3 * DEFAULT_WIDGET_Y_OFFSET + 1)
            .setSize(100, 24)
            .setDisplayCondition([this]() { return this->clientState == ClientState::MENU; })
            .build()
        }
    );

    //// Game Room Widgets ////
    // Start game button
    widgets.insert({
        "start", ButtonWidget::builder(
            "Start",
            [this]() {
                this->startGame(true);
            })
        .setPosition(GAME_ROOM_POSITION.x, GAME_ROOM_POSITION.y + 5 * DEFAULT_WIDGET_Y_OFFSET - 8.0f)
        .setTextSize(26)
        .setHeight(40)
        .setDisplayCondition([this]() { return this->hosting && this->clientState == ClientState::GAME_ROOM; })
        .build()
    });

    // Disconnect button
    widgets.insert({
        "disconnect",
        ButtonWidget::builder(
            "Disconnect",
            [this]() {
                this->disconnect();
            })
        .setPosition(GAME_ROOM_POSITION.x + 141.0f, GAME_ROOM_POSITION.y + 5 * DEFAULT_WIDGET_Y_OFFSET - 8.0f)
        .setTextSize(26)
        .setSize(201, 40)
        .setDisplayCondition([this]() { return this->clientState == ClientState::GAME_ROOM; })
        .build()
    });

    // +- buttons for boardSize - validated on the server
    widgets.insert({
        "board_size_plus",
        ButtonWidget::builder(
            "+",
            [this]() {
                this->requestBoardSettingsUpdate(
                    boardData.boardSize + 1,
                    boardData.winConditionLength);
            })
        .setPosition(212, GAME_ROOM_POSITION.y + 2 * DEFAULT_WIDGET_Y_OFFSET + 1)
        .setSize(24, 24)
        .setDisplayCondition([this]() { return this->hosting && this->clientState == ClientState::GAME_ROOM; })
        .build()
    });

    widgets.insert({
        "board_size_minus",
        ButtonWidget::builder(
            "-",
            [this]() {
                this->requestBoardSettingsUpdate(
                    boardData.boardSize - 1,
                    boardData.winConditionLength);
            })
        .setPosition(242, GAME_ROOM_POSITION.y + 2 * DEFAULT_WIDGET_Y_OFFSET + 1)
        .setSize(24, 24)
        .setDisplayCondition([this]() { return this->hosting && this->clientState == ClientState::GAME_ROOM; })
        .build()
    });

    //+- buttons for win condition length - validated on the server
    widgets.insert({
        "win_condition_plus",
        ButtonWidget::builder(
            "+",
            [this]() {
                this->requestBoardSettingsUpdate(
                    boardData.boardSize,
                    boardData.winConditionLength + 1);
            })
        .setPosition(294, GAME_ROOM_POSITION.y + 3 * DEFAULT_WIDGET_Y_OFFSET + 1)
        .setSize(24, 24)
        .setDisplayCondition([this]() { return this->hosting && this->clientState == ClientState::GAME_ROOM; })
        .build()
    });

    widgets.insert({
        "win_condition_minus",
        ButtonWidget::builder(
            "-",
            [this]() {
                this->requestBoardSettingsUpdate(
                    boardData.boardSize,
                    boardData.winConditionLength - 1);
            })
        .setPosition(324, GAME_ROOM_POSITION.y + 3 * DEFAULT_WIDGET_Y_OFFSET + 1)
        .setSize(24, 24)
        .setDisplayCondition([this]() { return this->hosting && this->clientState == ClientState::GAME_ROOM; })
        .build()
    });

    //// Game Widgets ////
    // Play again button
    widgets.insert({
        "play_again",
        ButtonWidget::builder(
            "Play Again",
            [this]() {
                //Send a game start request
                this->startGame(false);
            })
        .setPosition(
            WIN_TEXT_DRAW_AREA.position.x + WIN_TEXT_DRAW_AREA.size.x / 2.0f - 350.0f,
            WIN_TEXT_DRAW_AREA.position.y + WIN_TEXT_DRAW_AREA.size.y - 70.0f)
        .setSize(300.0f, 50.0f)
        .setTextSize(24)
        .setDisplayCondition([this]() {
            return this->hosting && this->gamePhase == GamePhase::GAME_FINISHED && this->clientState ==
                   ClientState::GAME;
        })
        .build()
    });

    // Back to game room button
    widgets.insert({
        "back_to_game_room",
        ButtonWidget::builder(
            "Back to the Game Room",
            [this]() {
                BackToGameRoomPacket backPacket{};
                backPacket.playerId = playerId;

                this->networkManager.sendPacket(PacketType::BACK_TO_GAME_ROOM, backPacket);
            })
        .setPosition(
            WIN_TEXT_DRAW_AREA.position.x + WIN_TEXT_DRAW_AREA.size.x / 2.0f + 50.0f,
            WIN_TEXT_DRAW_AREA.position.y + WIN_TEXT_DRAW_AREA.size.y - 70.0f)
        .setSize(300.0f, 50.0f)
        .setTextSize(24)
        .setDisplayCondition([this]() {
            return this->hosting && this->gamePhase == GamePhase::GAME_FINISHED && this->clientState ==
                   ClientState::GAME;
        })
        .build()
    });
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

        // Handle buttons
        // TODO: [idea] Maybe iterate backwards to let top buttons capture events first?
        for (const auto &widget: widgets | std::views::values) {
            if (widget->handleEvent(event, mousePos)) {
                // The event got consumed, so we can stop processing it and go to the next
                break;
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

/**
 * This function originally handled inputs for widgets too, but after unifying them, it's a leftover.
 * Left as is, as it may become useful later on
 */
void GameClient::handleMenuInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) {
    // Input that is not a widget goes here
    return;
}

/**
 * This function originally handled inputs for widgets too, but after unifying them, it's a leftover.
 * Left as is as, it may become useful later on
 */
void GameClient::handleGameRoomInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) {
    // Input that is not a widget goes here
    return;
}

void GameClient::handleGameInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) {
    if (const auto &btnEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
        if (isMyTurn && btnEvent->button == sf::Mouse::Button::Left) {
            const sf::Vector2i gridPos = BoardRenderer::getSquareAt(mousePos, boardData, BOARD_DRAW_AREA);

            if ((gridPos.x != -1 || gridPos.y != -1) && gamePhase != GamePhase::GAME_FINISHED) {
                printf(ANSI_GREEN "[GameClient] Clicked square at: [%d, %d]\n" ANSI_RESET, gridPos.x, gridPos.y);
                this->sendMove(gridPos.x, gridPos.y);
            }
        }
    }
}

void GameClient::update() {
    const sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    const sf::Time deltaTime = clock.restart();

    //Update buttons
    for (const auto &widget: widgets | std::views::values) {
        widget->update(deltaTime, mousePos);
    }

    // Networking
    PacketHeader header{};
    std::vector<char> payload;

    while (networkManager.pollPacket(header, payload)) {
        //S2C packets: SERVER_HELLO[x], SETUP_ACK[x], NEW_PLAYER_JOIN[x], SETTINGS_UPDATE[x], PLAYER_DISCONNECTED[x], GAME_START[x], BOARD_STATE_UPDATE[x], BACK_TO_GAME_ROOM[x], GAME_END[x]
        switch (header.type) {
            default: {
                printf(ANSI_RED "[GameClient] Unknown packet received! Type: %hhd\n", header.type);
                break;
            }

            case PacketType::SERVER_HELLO: {
                printf(ANSI_CYAN "[GameClient] Got a Server Hello packet!\n" ANSI_RESET);

                if (clientState != ClientState::GAME_ROOM) {
                    printf(
                        ANSI_RED "Client isn't in the game room state, but we received a SERVER_HELLO packet\n"
                        ANSI_RESET);
                }

                const auto *packet = reinterpret_cast<ServerHelloPacket *>(payload.data());
                this->handleServerHelloPacket(packet);

                break;
            }

            case PacketType::SETUP_ACK: {
                printf(ANSI_CYAN "[GameClient] Got a SETUP_ACK packet!\n" ANSI_RESET);

                if (clientState != ClientState::GAME_ROOM) {
                    printf(
                        ANSI_RED "Client isn't in the game room state, but we received a SETUP_ACK packet\n"
                        ANSI_RESET);
                }

                const auto *packet = reinterpret_cast<SetupAckPacket *>(payload.data());
                this->handleSetupAckPacket(packet);

                break;
            }

            case PacketType::NEW_PLAYER_JOIN: {
                printf(ANSI_CYAN "[GameClient] Got a NEW_PLAYER_JOIN packet!\n" ANSI_RESET);

                if (clientState != ClientState::GAME_ROOM) {
                    printf(
                        ANSI_RED "Client isn't in the game room state, but we received a NEW_PLAYER_JOIN packet\n"
                        ANSI_RESET);
                }

                const auto *packet = reinterpret_cast<NewPlayerJoinPacket *>(payload.data());
                this->handleNewPlayerJoinPacket(packet);

                break;
            }

            case PacketType::SETTINGS_UPDATE: {
                printf(ANSI_CYAN "[GameClient] Got a SETTINGS_UPDATE packet!\n" ANSI_RESET);

                const auto *packet = reinterpret_cast<SettingsUpdatePacket *>(payload.data());
                this->handleSettingsUpdatePacket(packet);

                break;
            }

            case PacketType::PLAYER_DISCONNECTED: {
                printf(ANSI_CYAN "[GameClient] Got a PLAYER_DISCONNECTED packet!\n" ANSI_RESET);

                const auto *packet = reinterpret_cast<PlayerDisconnectedPacket *>(payload.data());
                this->handlePlayerDisconnectedPacket(packet);

                break;
            }

            case PacketType::GAME_START: {
                printf(ANSI_CYAN "[GameClient] Got a GAME_START packet!\n" ANSI_RESET);

                const auto *packet = reinterpret_cast<GameStartPacket *>(payload.data());
                this->handleGameStartPacket(packet);

                break;
            }
            case PacketType::BOARD_STATE_UPDATE: {
                printf(ANSI_CYAN "[GameClient] Got a BOARD_STATE_UPDATE packet!\n" ANSI_RESET);

                const auto *packet = reinterpret_cast<BoardStateUpdatePacket *>(payload.data());
                if (this->handleBoardStateUpdatePacket(packet)) break;

                break;
            }

            case PacketType::BACK_TO_GAME_ROOM: {
                printf(ANSI_CYAN "[GameClient] Got a BACK_TO_GAME_ROOM packet, going back.\n" ANSI_RESET);

                clientState = ClientState::GAME_ROOM;
                gamePhase = GamePhase::WAITING_ROOM;

                break;
            }

            case PacketType::GAME_END: {
                printf(ANSI_CYAN "[GameClient] Got a GAME_END packet!\n" ANSI_RESET);

                const auto *packet = reinterpret_cast<GameEndPacket *>(payload.data());
                this->handleGameEndPacket(packet);

                break;
            }
        }
    }
}

void GameClient::handleServerHelloPacket(const ServerHelloPacket *packet) {
    setupPhase = SetupPhase::SETTING_UP;
    playerId = packet->playerId;
    printf(ANSI_CYAN "[GameClient] Assigned player ID: %hhu\n" ANSI_RESET, packet->playerId);

    // Send the SETUP_REQ packet
    SetupReqPacket setupReqPacket{};
    setupReqPacket.playerId = playerId;
    memset(setupReqPacket.playerName, 0, MAX_PLAYER_NAME_LENGTH);
    strncpy(setupReqPacket.playerName, playerName.c_str(), MAX_PLAYER_NAME_LENGTH - 1);
    setupReqPacket.initialToken = initialToken;
    setupReqPacket.isHost = hosting;

    printf(
        ANSI_CYAN "[GameClient] Sending SETUP_REQ packet with [pid:%hhu] [pName:%s] [initialToken:%d]\n"
        ANSI_RESET, playerId, playerName.c_str(), initialToken);
    networkManager.sendPacket<SetupReqPacket>(PacketType::SETUP_REQ, setupReqPacket);
    printf(ANSI_CYAN "[GameClient] SETUP_REQ sent!\n");
}

void GameClient::handleSetupAckPacket(const SetupAckPacket *packet) {
    if (playerId != packet->playerId) {
        printf(
            ANSI_RED
            "[GameClient] Somehow got SETUP_ACK dedicated to another player! This shouldn't happen!\n"
            ANSI_RESET);
    }

    authToken = packet->generatedAuthToken;
    pieceType = packet->pieceType;
    //This technically isn't necessary, but I should at least check if the server is happy with the chosen name
    playerName = packet->playerName;

    boardData.boardSize = packet->boardSize;
    boardData.winConditionLength = packet->winConditionLength;
    boardData.round = packet->round;

    playerCount = packet->playerCount;
    players = {};
    for (const auto player: packet->players) {
        if (player.playerId == 0) {
            continue;
        }
        printf("[%hhu, %s, %hhd, %hhd, %hhd]\n", player.playerId, player.playerName, player.piece,
               player.myTurn, player.isMe);
        players.push_back(player);
    }

    setupPhase = SetupPhase::CONNECTED;
    gamePhase = GamePhase::WAITING_ROOM;
    printf(ANSI_CYAN "[GameClient] Generated AuthToken: %d Other: [name:%s, id:%hhd]\n" ANSI_RESET,
           authToken, playerName.c_str(), pieceType);
}

void GameClient::handleNewPlayerJoinPacket(const NewPlayerJoinPacket *packet) {
    Player newPlayer{};
    newPlayer.playerId = packet->newPlayerId;
    memset(newPlayer.playerName, 0, MAX_PLAYER_NAME_LENGTH);
    strncpy(newPlayer.playerName, packet->newPlayerName, MAX_PLAYER_NAME_LENGTH - 1);
    newPlayer.piece = packet->newPlayerPieceType;
    newPlayer.isMe = playerId == packet->newPlayerId;
    newPlayer.myTurn = false;
    newPlayer.wins = 0;
    newPlayer.isHost = packet->isHost;

    // If a player with this id already exists overwrite it
    std::erase_if(
        players, [packet](const Player &player) { return packet->newPlayerId == player.playerId; });
    players.push_back(newPlayer);
    playerCount = players.size();
}

void GameClient::handleSettingsUpdatePacket(const SettingsUpdatePacket *packet) {
    printf(ANSI_GREEN "[GameClient] New Board Size: %hhu, New Win Condition Length: %hhu\n" ANSI_RESET,
           packet->newBoardSize, packet->newWinConditionLength);

    boardData.boardSize = packet->newBoardSize;
    boardData.winConditionLength = packet->newWinConditionLength;
}

void GameClient::handlePlayerDisconnectedPacket(const PlayerDisconnectedPacket *packet) {
    printf(ANSI_YELLOW "[GameClient] Player with ID %hhu has disconnected\n" ANSI_RESET, packet->playerId);

    std::erase_if(players, [packet](const Player &player) {
        return player.playerId == packet->playerId;
    });
    playerCount = players.size();
}

void GameClient::handleGameStartPacket(const GameStartPacket *packet) {
    printf(ANSI_GREEN "[GameClient] The Game is starting... Started by player with ID %hhu\n" ANSI_RESET,
           packet->requestedByPlayerId);

    boardData.boardSize = packet->finalBoardSize;
    boardData.winConditionLength = packet->finalWinConditionLength;
    boardData.round = packet->round;
    boardData.turn = packet->turn;
    boardData.actingPlayerId = packet->startingPlayerId;
    for (auto &player: players) {
        if (player.playerId == packet->startingPlayerId) {
            player.myTurn = true;
        }
    }

    Utils::initializeGameBoard(boardData);
    Utils::deserializeBoard(packet->grid, boardData);

    clientState = ClientState::GAME;
    gamePhase = (playerId == packet->startingPlayerId ? GamePhase::MY_TURN : GamePhase::NOT_MY_TURN);
    isMyTurn = playerId == packet->startingPlayerId;
}

bool GameClient::handleBoardStateUpdatePacket(const BoardStateUpdatePacket *packet) {
    if (clientState != ClientState::GAME) {
        printf(
            ANSI_RED "Client isn't in the game state, but we received a BOARD_STATE_UPDATE packet\n"
            ANSI_RESET);
        return true;
    }

    //update board
    Utils::deserializeBoard(packet->grid, boardData);

    boardData.round = packet->round;
    boardData.turn = packet->turn;
    boardData.actingPlayerId = packet->actingPlayerId;
    lastMove = packet->lastMove;
    moves.push_back(packet->lastMove);

    isMyTurn = playerId == packet->actingPlayerId;

    //update players
    players.clear();
    for (int i = 0; i < packet->playerCount; ++i) {
        players.push_back(packet->players[i]);
    }
    return false;
}

void GameClient::handleGameEndPacket(const GameEndPacket *packet) {
    printf(ANSI_GREEN "[GameClient] Player with ID %hhu finished the round!\n" ANSI_RESET,
           packet->playerId);

    gamePhase = GamePhase::GAME_FINISHED;
    finishReason = packet->reason;
    gameEndPlayer = packet->player;
    std::erase_if(players, [packet](const Player &player) {
        return player.playerId == packet->playerId;
    });

    if (packet->reason != FinishReason::PLAYER_DISCONNECT) {
        players.push_back(packet->player);
    }
}

void GameClient::render() {
    window.clear(sf::Color(BACKGROUND_COLOR));

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

    for (const auto &widget: widgets | std::views::values) {
        widget->render(window);
    }

    if (debugEnabled) {
        this->renderDebugMenu();
    }

    window.display();
}

void GameClient::renderMenu() {
    sf::Text text(font);

    text.setString((hosting ? "You are the host!" : "Hello there!"));
    text.setCharacterSize(DEFAULT_TEXT_SIZE);
    text.setFillColor(sf::Color(TEXT_COLOR));
    text.setPosition({MAIN_MENU_POSITION.x, MAIN_MENU_POSITION.y});
    window.draw(text);

    text.setString("Player name:"); // + playerName
    text.move({0, DEFAULT_WIDGET_Y_OFFSET});
    window.draw(text);

    text.setString("Server Address:"); // + userInputIP
    text.move({0, DEFAULT_WIDGET_Y_OFFSET});
    window.draw(text);
}

void GameClient::renderGameRoom() {
    sf::Text text(font);

    // Player name
    text.setString("Player name: " + playerName);
    text.setCharacterSize(DEFAULT_TEXT_SIZE);
    text.setFillColor(sf::Color(TEXT_COLOR));
    text.setPosition({GAME_ROOM_POSITION.x, GAME_ROOM_POSITION.y});
    window.draw(text);

    // Am I the Host
    text.setString((hosting ? "You are the host!" : "Someone else is hosting!"));
    text.move({0, DEFAULT_WIDGET_Y_OFFSET});
    window.draw(text);

    // Board Size
    text.setString("Board Size: " + std::to_string(boardData.boardSize));
    text.move({0, DEFAULT_WIDGET_Y_OFFSET});
    window.draw(text);

    // Win Condition Length
    text.setString("Required in a row: " + std::to_string(boardData.winConditionLength));
    text.move({0, DEFAULT_WIDGET_Y_OFFSET});
    window.draw(text);

    // space
    text.move({0, DEFAULT_WIDGET_Y_OFFSET});

    // Start button for the host - initialized in the button widgets
    text.move({0, DEFAULT_WIDGET_Y_OFFSET});

    // space
    text.move({0, DEFAULT_WIDGET_Y_OFFSET});

    // Players
    text.setString("Players[" + std::to_string(players.size()) + ", " + std::to_string(playerCount) + "]:");
    text.move({0, DEFAULT_WIDGET_Y_OFFSET});
    window.draw(text);

    for (const auto &player: players) {
        std::string playerString = "Name: ";
        playerString.append(player.playerName)
                .append("  ID: ").append(std::to_string(player.playerId))
                .append("  Piece: ").append(Utils::pieceTypeToString(player.piece))
                .append(player.isHost ? "  HOST" : "");

        text.setString(playerString);
        text.move({0, DEFAULT_WIDGET_Y_OFFSET});
        window.draw(text);
    }
}

void GameClient::renderGame() {
    const sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    const sf::Vector2i hoveredGridPos = BoardRenderer::getSquareAt(mousePos, boardData, BOARD_DRAW_AREA);

    BoardRenderer::render(window, boardData, BOARD_DRAW_AREA, isMyTurn, hoveredGridPos);

    //TODO: Add a banner saying what piece you are playing
    sf::Text playingAsText(font);
    playingAsText.setString("You're playing as " + Utils::pieceTypeToString(pieceType));
    playingAsText.setCharacterSize(28);
    playingAsText.setFillColor(sf::Color(TEXT_COLOR));
    DrawUtils::centerTextHorizontally(playingAsText);
    playingAsText.setPosition({
        std::floor(WIN_TEXT_DRAW_AREA.position.x + WIN_TEXT_DRAW_AREA.size.x / 2.0f),
        std::floor(WIN_TEXT_DRAW_AREA.position.y - WIN_TEXT_DRAW_AREA.size.y / 2.0f)
    });
    window.draw(playingAsText);

    if (gamePhase == GamePhase::GAME_FINISHED) {
        sf::RectangleShape rect(WIN_TEXT_DRAW_AREA.size);
        rect.setPosition(WIN_TEXT_DRAW_AREA.position);
        rect.setFillColor(sf::Color(BACKGROUND_COLOR));
        window.draw(rect);

        sf::Text gameEndText(font);
        gameEndText.setCharacterSize(28);
        gameEndText.setFillColor(sf::Color(TEXT_COLOR));

        switch (finishReason) {
            default: {
                gameEndText.setString("Game Ended Reason Unknown");
                break;
            }

            case FinishReason::PLAYER_WIN: {
                // Player {} playing {} has won the round!
                std::string winString = "Player[";
                winString.append(std::to_string(gameEndPlayer.playerId)).append("] ")
                        .append(gameEndPlayer.playerName)
                        .append(" playing ").append(Utils::pieceTypeToString(gameEndPlayer.piece))
                        .append(" has won the round!");
                gameEndText.setString(winString);
                break;
            }

            case FinishReason::PLAYER_DISCONNECT: {
                // Player {} has disconnected
                std::string disconnectString = "Player[";
                disconnectString.append(std::to_string(gameEndPlayer.playerId)).append("] ")
                        .append(gameEndPlayer.playerName)
                        .append(" playing ").append(Utils::pieceTypeToString(gameEndPlayer.piece))
                        .append(" has disconnected!");
                gameEndText.setString(disconnectString);
                break;
            }
        }

        DrawUtils::centerText(gameEndText);
        gameEndText.setPosition({
            std::floor(WIN_TEXT_DRAW_AREA.position.x + WIN_TEXT_DRAW_AREA.size.x / 2.0f) + 1.0f,
            std::floor(WIN_TEXT_DRAW_AREA.position.y + WIN_TEXT_DRAW_AREA.size.y / 7.0f) - 4.0f
        });
        window.draw(gameEndText);

        sf::Text scoreBoardText(font);
        scoreBoardText.setCharacterSize(24);
        scoreBoardText.setFillColor(sf::Color(TEXT_COLOR));
        scoreBoardText.setString("Score Board");
        DrawUtils::centerText(scoreBoardText);
        scoreBoardText.setPosition({
            std::floor(WIN_TEXT_DRAW_AREA.position.x + WIN_TEXT_DRAW_AREA.size.x / 2.0f) + 1.0f,
            std::floor(WIN_TEXT_DRAW_AREA.position.y + WIN_TEXT_DRAW_AREA.size.y / 7.0f) - 4.0f +
            DEFAULT_WIDGET_Y_OFFSET + 8.0f
        });
        window.draw(scoreBoardText);


        int offset = 1;
        for (const auto &player: players) {
            sf::Text scoreText(font);
            scoreText.setCharacterSize(DEFAULT_TEXT_SIZE);
            scoreText.setFillColor(sf::Color(TEXT_COLOR));

            std::string pNameString = player.playerName;
            std::string scoreString = playerName + " " + Utils::pieceTypeToString(player.piece) + " Wins: " +
                                      std::to_string(player.wins);
            scoreText.setString(scoreString);

            DrawUtils::centerText(scoreText);
            scoreText.setPosition({
                std::floor(WIN_TEXT_DRAW_AREA.position.x + WIN_TEXT_DRAW_AREA.size.x / 2.0f) + 1.0f,
                std::floor(WIN_TEXT_DRAW_AREA.position.y + WIN_TEXT_DRAW_AREA.size.y / 7.0f) - 4.0f +
                DEFAULT_WIDGET_Y_OFFSET + 12.0f + ((DEFAULT_WIDGET_Y_OFFSET - 6.0f) * offset)
            });

            window.draw(scoreText);
            ++offset;
        }


        if (!hosting) {
            sf::Text waitingForHostText(font);
            waitingForHostText.setCharacterSize(24);
            waitingForHostText.setFillColor(sf::Color(TEXT_COLOR));
            waitingForHostText.setString("Waiting for the Host");

            DrawUtils::centerText(waitingForHostText);
            waitingForHostText.setPosition({
                WIN_TEXT_DRAW_AREA.position.x + WIN_TEXT_DRAW_AREA.size.x / 2.0f,
                WIN_TEXT_DRAW_AREA.position.y + WIN_TEXT_DRAW_AREA.size.y - 30.0f
            });
            window.draw(waitingForHostText);
        }
    }
}

void GameClient::renderDebugMenu() {
    const sf::Vector2u debugMenuPosition{window.getSize().x - 500, 16}; //left corner
    constexpr int textSize = 14;
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
        case PieceType::OCTAGON:
            pieceTypeText += "[5] Octagon";
            break;
        case PieceType::HEXAGON:
            pieceTypeText += "[6] Hexagon";
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
        case GamePhase::GAME_FINISHED:
            gamePhaseString += "[4] Game Finished";
            break;
    }
    text.setString(gamePhaseString);
    text.move({0, textYOffset});
    window.draw(text);

    std::string finishReasonString = "Finish Reason: ";
    switch (finishReason) {
        case FinishReason::NONE:
            finishReasonString += "[0] None";
            break;
        case FinishReason::PLAYER_WIN:
            finishReasonString += "[1] Player Win";
            break;
        case FinishReason::PLAYER_DISCONNECT:
            finishReasonString += "[2] Player Disconnect";
            break;
        case FinishReason::OTHER:
            finishReasonString += "[3] Other";
            break;
    }
    text.setString(finishReasonString);
    text.move({0, textYOffset});
    window.draw(text);

    std::string playerString = std::format("Players[{}]: [", players.size());
    for (auto player: players) {
        std::stringstream ss;
        ss << "{"
                << static_cast<int>(player.playerId) << ", "
                << player.playerName << ", "
                << static_cast<int>(player.piece)
                << ", W" << static_cast<int>(player.wins)
                << (player.isHost ? ", H" : "")
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

        std::string tickString = std::format(
            "Tick: {} Avg: {:.2f}ms Last: {:.2f}ms",
            serverLogic.getTick(),
            serverLogic.getAvgTickTime() / 1000000.0,
            serverLogic.getLastTickTime() / 1000000.0
        );
        text.setString(tickString);
        text.move({0, textYOffset});
        window.draw(text);

        //server port
        text.setString("ServerPort: " + std::to_string(serverLogic.getServerPort()));
        text.move({0, textYOffset});
        window.draw(text);

        //next player id
        text.setString("NextPlayerID: " + std::to_string(serverLogic.getNextPlayerId()));
        text.move({0, textYOffset});
        window.draw(text);

        //turn
        text.setString("Turn: " + std::to_string(serverLogic.getCurrentTurn()));
        text.move({0, textYOffset});
        window.draw(text);

        //hosting player
        text.setString("HostingPlayerID: " + std::to_string(serverLogic.getHostingPlayerId()));
        text.move({0, textYOffset});
        window.draw(text);

        //game settings
        const auto &[srvBoardSize, srvWinConLength] = serverLogic.getBoardSettings();
        std::string gameSettings =
                "Game Settings[BoardSize: " + std::to_string(srvBoardSize) +
                ", WinConLength: " + std::to_string(srvWinConLength) + "]";
        text.setString(gameSettings);
        text.move({0, textYOffset});
        window.draw(text);

        //available pieces
        std::string availablePiecesString = "AvPieces[";
        for (const auto &piece: serverLogic.getAllAvailablePieces()) {
            availablePiecesString += Utils::pieceTypeToString(piece) + ",";
        }
        availablePiecesString += "]";
        text.setString(availablePiecesString);
        text.move({0, textYOffset});
        window.draw(text);

        //players
        std::string playersString = "Players[";
        for (const auto &player: serverLogic.getPlayers()) {
            std::stringstream ss;
            ss << "{"
                    << static_cast<int>(player.playerId) << ", "
                    << player.playerName << ", "
                    << static_cast<int>(player.piece) << ", w"
                    << static_cast<int>(player.wins) << ", "
                    << (player.myTurn ? "T" : "F") << "}";
            playersString += ss.str();
        }
        playersString += "]";
        text.setString(playersString);
        text.move({0, textYOffset});
        window.draw(text);

        //moves
        std::string moveString = "Moves[";
        for (const auto &move: serverLogic.getMoves()) {
            std::stringstream ss;
            ss << "{"
                    << static_cast<int>(move.playerId)
                    << ", X" << static_cast<int>(move.posX)
                    << ", Y" << static_cast<int>(move.posY)
                    << ", T" << static_cast<int>(move.turnPlaced)
                    << ", " << Utils::pieceTypeToString(move.piece)
                    << "}";
            moveString += ss.str();
        }
        moveString += "]";
        text.setString(moveString);
        text.move({0, textYOffset});
        window.draw(text);
    }
}

std::optional<std::pair<std::string, std::string> > GameClient::parseServerAddrAndPortFromTextField() const {
    //^((?:\D+).\w{2,8}|(?:\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))):(\d{1,5}\b)$
    // Examples:
    // 192.168.2.32:27015 <- valid -> Group 1: 192.168.2.32 Group 2: 27015
    // domain.example.com:27015 <- valid -> Group 1: domain.example.com Group 2: 27015
    // 129.212.913.123:12312 <- invalid
    // localhost:27015 <- valid -> Group 1: localhost Group 2: 27015
    static std::regex SERVER_IP_PATTERN(
        R"(^((?:\D+).\w{2,8}|(?:\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))):(\d{1,5}\b)$)");
    if (std::smatch matches; std::regex_match(userInputIP, matches, SERVER_IP_PATTERN)) {
        // serverAddress = matches[1].str();
        // serverPort = matches[2].str();

        return std::make_optional(std::make_pair(matches[1].str(), matches[2].str()));
    }

    printf(
        ANSI_RED "[GameClient] Please input a valid server address in the format: {ip/address}:{port}\n"
        ANSI_RESET);

    return std::nullopt;
}

void GameClient::connectAndSetup() {
    auto serverAddrOpt = this->parseServerAddrAndPortFromTextField();
    if (!serverAddrOpt.has_value()) {
        return;
    }

    serverAddress = serverAddrOpt.value().first;
    serverPort = serverAddrOpt.value().second;

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

void GameClient::disconnect() {
    this->networkManager.disconnect();
    setupPhase = SetupPhase::DISCONNECTED;
    gamePhase = GamePhase::UNKNOWN;
    clientState = ClientState::MENU;
}


void GameClient::requestBoardSettingsUpdate(const uint8_t newBoardSize, const uint8_t newWinConditionLength) {
    SettingsChangeReqPacket changeReq{};
    changeReq.playerId = playerId;
    changeReq.authToken = authToken;
    changeReq.newBoardSize = newBoardSize;
    changeReq.newWinConditionLength = newWinConditionLength;

    printf(
        ANSI_CYAN "[GameClient] Sending settings change request packet with [boardSize: %hhu, winLength: %hhu]\n"
        ANSI_RESET, newBoardSize, newWinConditionLength);
    this->networkManager.sendPacket(PacketType::SETTINGS_CHANGE_REQ, changeReq);
}


void GameClient::startGame(bool newGame) {
    printf(ANSI_CYAN "[GameClient] Sending Start Game Request!\n" ANSI_RESET);
    GameStartRequestPacket startReq{};
    startReq.requestingPlayerId = playerId;
    startReq.newGame = newGame;

    this->networkManager.sendPacket(PacketType::GAME_START_REQ, startReq);
}

void GameClient::sendMove(uint8_t posX, uint8_t posY) {
    printf(ANSI_GREEN "[GameClient] Sending move packet with [x:%hhu, y:%hhu]]\n" ANSI_RESET, posX, posY);
    MoveRequestPacket moveReq{};
    moveReq.playerId = playerId;
    moveReq.x = posX;
    moveReq.y = posY;
    moveReq.turn = boardData.turn;
    moveReq.piece = pieceType;

    this->networkManager.sendPacket(PacketType::MOVE_REQ, moveReq);
}

void GameClient::startInternalServerThread() {
    auto serverAddrOpt = this->parseServerAddrAndPortFromTextField();
    if (!serverAddrOpt.has_value()) {
        printf(ANSI_RED "[GameClient] Invalid address and port, can't start the server with this!\n" ANSI_RESET);
        return;
    }

    serverPort = serverAddrOpt.value().second;

    printf(ANSI_CYAN "[GameClient] Internal Server is starting...\n" ANSI_RESET);
    serverThread = std::thread([this]() {
        serverLogic.start(std::stoi(serverPort));
    });

    const auto widget = reinterpret_cast<TextFieldWidget *>(widgets["server_ip_input"].get());
    widget->setActive(false);
    widget->setText(std::format("localhost:{}", serverPort));
}

void GameClient::stopInternalServerThread() {
    printf(ANSI_YELLOW "[GameClient] Internal Server is stopping...\n" ANSI_RESET);

    const auto widget = reinterpret_cast<TextFieldWidget *>(widgets["server_ip_input"].get());
    widget->setActive(true);

    serverLogic.stop();

    if (serverThread.joinable()) {
        serverThread.join();
    }
}

GameClient::~GameClient() {
    if (hosting) {
        stopInternalServerThread();
    }
}



