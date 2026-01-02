#include "GameClient.h"

#include <cmath>
#include <ranges>
#include <thread>

#include "SFML/Graphics/Text.hpp"
#include "ui/BoardRenderer.h"
#include "ui/ButtonBuilder.h"

GameClient::GameClient() {
    clientState = ClientState::MENU;
    userInputIP = "localhost:27015";
    playerName = "Player";
    setupPhase = SetupPhase::DISCONNECTED;
    finishReason = FinishReason::NONE;

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
    boardData.actingPlayerId = 0;
    Utils::initializeGameBoard(boardData);
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
            .setPosition(MAIN_MENU_POSITION.x, MAIN_MENU_POSITION.y + 3 * DEFAULT_WIDGET_Y_OFFSET + 3)
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
            .setPosition(MAIN_MENU_POSITION.x, MAIN_MENU_POSITION.y + 2 * DEFAULT_WIDGET_Y_OFFSET + 1)
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
                this->startGame(true);
            })
        .setPosition(GAME_ROOM_POSITION.x, GAME_ROOM_POSITION.y + 5 * DEFAULT_WIDGET_Y_OFFSET)
        .setTextSize(26)
        .setDisplayCondition([this]() {return this->hosting;})
        .build()
    });

    // +- buttons for boardSize - validated on the server
    gameRoomButtons.insert({
        "boardsizeplus",
        ButtonWidget::builder(
            "+",
            [this]() {
                this->requestBoardSettingsUpdate(
                    boardData.boardSize + 1,
                    boardData.winConditionLength);
            })
        .setPosition(202, GAME_ROOM_POSITION.y + 2 * DEFAULT_WIDGET_Y_OFFSET + 1)
        .setSize(24, 24)
        .setDisplayCondition([this]() {return this->hosting;})
        .build()
    });

    gameRoomButtons.insert({
        "boardsizeminus",
        ButtonWidget::builder(
            "-",
            [this]() {
                this->requestBoardSettingsUpdate(
                    boardData.boardSize - 1,
                    boardData.winConditionLength);
            })
        .setPosition(232, GAME_ROOM_POSITION.y + 2 * DEFAULT_WIDGET_Y_OFFSET + 1)
        .setSize(24, 24)
        .setDisplayCondition([this]() {return this->hosting;})
        .build()
    });

    //+- buttons for win condition length - validated on the server
    gameRoomButtons.insert({
        "winconditionplus",
        ButtonWidget::builder(
            "+",
            [this]() {
                this->requestBoardSettingsUpdate(
                    boardData.boardSize,
                    boardData.winConditionLength + 1);
            })
        .setPosition(284, GAME_ROOM_POSITION.y + 3 * DEFAULT_WIDGET_Y_OFFSET + 1)
        .setSize(24, 24)
        .setDisplayCondition([this]() {return this->hosting;})
        .build()
    });

    gameRoomButtons.insert({
        "winconditionminus",
        ButtonWidget::builder(
            "-",
            [this]() {
                this->requestBoardSettingsUpdate(
                    boardData.boardSize,
                    boardData.winConditionLength - 1);
            })
        .setPosition(314, GAME_ROOM_POSITION.y + 3 * DEFAULT_WIDGET_Y_OFFSET + 1)
        .setSize(24, 24)
        .setDisplayCondition([this]() {return this->hosting;})
        .build()
    });
}

void GameClient::initGameWidgets() {
    //TODO:
    gameButtons.insert({
        "playagain",
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
        .setDisplayCondition([this]() {return this->hosting && this->gamePhase == GamePhase::GAME_FINISHED;})
        .build()
    });

    gameButtons.insert({
        "backtoroom",
        ButtonWidget::builder(
            "Back to the Game Room",
            [this]() {
                BackToGameRoomPacket backPacket {};
                backPacket.playerId = playerId;

                this->networkManager.sendPacket(PacketType::BACK_TO_GAME_ROOM, backPacket);
            })
        .setPosition(
            WIN_TEXT_DRAW_AREA.position.x + WIN_TEXT_DRAW_AREA.size.x / 2.0f + 50.0f,
            WIN_TEXT_DRAW_AREA.position.y + WIN_TEXT_DRAW_AREA.size.y - 70.0f)
        .setSize(300.0f, 50.0f)
        .setTextSize(24)
        .setDisplayCondition([this]() {return this->hosting && this->gamePhase == GamePhase::GAME_FINISHED;})
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

    //Update buttons //TODO: simplify this further?
    for (const auto &btn: menuButtons | std::views::values) {
        btn->update(mousePos);
    }

    for (const auto &btn: gameRoomButtons | std::views::values) {
        btn->update(mousePos);
    }

    for (const auto &btn: gameButtons | std::views::values) {
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
    for (const auto &btn: gameRoomButtons | std::views::values) {
        btn->handleEvent(event, mousePos);
    }
}

void GameClient::handleGameInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) {
    if (const auto &btnEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
        if (isMyTurn && btnEvent->button == sf::Mouse::Button::Left) {
            sf::Vector2i gridPos = BoardRenderer::getSquareAt(mousePos, boardData, BOARD_DRAW_AREA);

            if ((gridPos.x != -1 || gridPos.y != -1) && gamePhase != GamePhase::GAME_FINISHED) {
                printf(ANSI_GREEN "[GameClient] Clicked square at: [%d, %d]\n" ANSI_RESET, gridPos.x, gridPos.y);
                this->sendMove(gridPos.x, gridPos.y);
            }
        }
    }

    for (const auto &btn: gameButtons | std::views::values) {
        btn->handleEvent(event, mousePos);
    }
}

void GameClient::update() {
    PacketHeader header{};
    std::vector<char> payload;

    while (networkManager.pollPacket(header, payload)) {
        //S2C packets: SERVER_HELLO[x], SETUP_ACK[x], NEW_PLAYER_JOIN[x], SETTINGS_UPDATE[x], GAME_START, BOARD_STATE_UPDATE, GAME_END
        switch (header.type) {
            default: {
                printf(ANSI_RED "[GameClient] Unknown packet received! Type: %hhd, Data: [%s]", header.type, &payload);
                break;
            }
            case PacketType::SERVER_HELLO: {
                if (clientState != ClientState::GAME_ROOM) {
                    printf(
                        ANSI_RED "Client isnt in the game room state, but we received a SERVER_HELLO packet\n"
                        ANSI_RESET);
                }
                printf(ANSI_CYAN "[GameClient] Got a Server Hello packet!\n" ANSI_RESET);
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
                setupReqPacket.isHost = hosting;

                printf(
                    ANSI_CYAN "[GameClient] Sending SETUP_REQ packet with [pid:%hhu] [pName:%s] [initialToken:%d]\n"
                    ANSI_RESET, playerId, playerName.c_str(), initialToken);
                networkManager.sendPacket<SetupReqPacket>(PacketType::SETUP_REQ, setupReqPacket);
                printf(ANSI_CYAN "[GameClient] SETUP_REQ sent!\n");
                break;
            }

            case PacketType::SETUP_ACK: {
                if (clientState != ClientState::GAME_ROOM) {
                    printf(
                        ANSI_RED "Client isn't in the game room state, but we received a SETUP_ACK packet\n"
                        ANSI_RESET);
                }
                printf(ANSI_CYAN "[GameClient] Got a SETUP_ACK packet!\n" ANSI_RESET);
                //TODO: check if the playerId is the same as ours
                const auto *packet = reinterpret_cast<SetupAckPacket *>(payload.data());
                authToken = packet->generatedAuthToken;
                pieceType = packet->pieceType;
                //This technically isn't necessary, but I should at least check if the server is happy with the chosen name
                playerName = packet->playerName;

                boardData.boardSize = packet->boardSize;
                boardData.winConditionLength = packet->winConditionLength;
                boardData.round = packet->round;

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
                gamePhase = GamePhase::WAITING_ROOM;
                printf(ANSI_CYAN "[GameClient] Generated AuthToken: %d Other: [name:%s, id:%hhd]\n" ANSI_RESET,
                       authToken, playerName.c_str(), pieceType);
                break;
            }

            case PacketType::NEW_PLAYER_JOIN: {
                if (clientState != ClientState::GAME_ROOM) {
                    printf(
                        ANSI_RED "Client isn't in the game room state, but we received a NEW_PLAYER_JOIN packet\n"
                        ANSI_RESET);
                }

                //TODO: Check for duplicates and check if its me(The player requesting the setup exchange)
                printf(ANSI_CYAN "[GameClient] Got a NEW_PLAYER_JOIN packet!\n" ANSI_RESET);
                const auto *packet = reinterpret_cast<NewPlayerJoinPacket *>(payload.data());

                Player newPlayer{};
                newPlayer.playerId = packet->newPlayerId;
                // newPlayer.playerName = packet->newPlayerName;
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
                break;
            }

            case PacketType::SETTINGS_UPDATE: {
                const auto *packet = reinterpret_cast<SettingsUpdatePacket *>(payload.data());
                printf(ANSI_CYAN "[GameClient] Got a SETTINGS_UPDATE packet!\n" ANSI_RESET);
                printf(ANSI_GREEN "[GameClient] New Board Size: %hhu, New Win Condition Length: %hhu\n" ANSI_RESET,
                       packet->newBoardSize, packet->newWinConditionLength);
                boardData.boardSize = packet->newBoardSize;
                boardData.winConditionLength = packet->newWinConditionLength;
                break;
            }

            case PacketType::PLAYER_DISCONNECTED: {
                const auto *packet = reinterpret_cast<PlayerDisconnectedPacket *>(payload.data());
                printf(ANSI_YELLOW "[GameClient] Player with ID %hhu has disconnected\n" ANSI_RESET, packet->playerId);
                //TODO: handle disconnect?? pause game?? Check if the disconnected player is me - this shouldn't happen
                std::erase_if(players, [packet](const Player &player) {
                    return player.playerId == packet->playerId;
                });
                playerCount = players.size();
                break;
            }

            case PacketType::GAME_START: {
                const auto *packet = reinterpret_cast<GameStartPacket *>(payload.data());
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

                // std::string boardString = "Grid: [";
                // for (auto &row : boardData.grid) {
                //     for (auto &column : row) {
                //         boardString += Utils::pieceTypeToString(column.piece) + ",";
                //     }
                // }
                // boardString += "]";
                // printf("%s\n", boardString.c_str());

                break;
            }
            case PacketType::BOARD_STATE_UPDATE: {
                const auto *packet = reinterpret_cast<BoardStateUpdatePacket *>(payload.data());
                printf(ANSI_CYAN "[GameClient] Got a BOARD_STATE_UPDATE packet!\n" ANSI_RESET);
                if (clientState != ClientState::GAME) {
                    printf(
                        ANSI_RED "Client isn't in the game state, but we received a BOARD_STATE_UPDATE packet\n"
                        ANSI_RESET);
                    break;
                }

                //update board
                //TODO: check if gamesettings still match
                Utils::deserializeBoard(packet->grid, boardData);

                //Debug
                // std::string boardString = "Grid: [";
                // for (auto &row : boardData.grid) {
                //     for (auto &column : row) {
                //         boardString += Utils::pieceTypeToString(column.piece) + ",";
                //     }
                // }
                // boardString += "]";
                // printf("[GameClient] %s\n", boardString.c_str());
                ////////////

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
                break;
            }

            case PacketType::BACK_TO_GAME_ROOM: {
                printf(ANSI_CYAN "[GameClient] Got a BACK_TO_GAME_ROOM packet, going back.\n" ANSI_RESET);
                clientState = ClientState::GAME_ROOM;
                gamePhase = GamePhase::WAITING_ROOM;
                break;
            }

            case PacketType::GAME_END: {
                const auto *packet = reinterpret_cast<GameEndPacket *>(payload.data());
                //TODO: check for win bool, if it wasnt a premature end because of an error or smth
                printf(ANSI_GREEN "[GameClient] Player with ID %hhu won the round!\n" ANSI_RESET,
                       packet->playerId);

                //TODO: a banner with the winning player, a button to restart and a button to get back to the game_room

                // gamePhase = GamePhase::WAITING_ROOM;
                // clientState = ClientState::GAME_ROOM;
                gamePhase = GamePhase::GAME_FINISHED;
                finishReason = packet->reason;
                gameEndPlayer = packet->player;
                std::erase_if(players, [packet](const Player &player) {
                    return player.playerId == packet->playerId;
                });
                players.push_back(packet->player);
                // const auto it = std::ranges::find_if(players, [packet](const Player &p) {return p.playerId == packet->playerId;});
                // if (it == players.end()) {
                //     // Couldn't find any matching players
                //     break;
                // }
                // winningPlayer = *it;

                //TODO: display something

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
    text.setCharacterSize(DEFAULT_TEXT_SIZE);
    text.setFillColor(sf::Color(TEXT_COLOR));
    text.setPosition({MAIN_MENU_POSITION.x, MAIN_MENU_POSITION.y});
    window.draw(text);

    text.setString("Player name: " + playerName);
    text.move({0, DEFAULT_WIDGET_Y_OFFSET});
    window.draw(text);

    text.setString("Server Address: " + userInputIP);
    text.move({110, DEFAULT_WIDGET_Y_OFFSET});
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
        std::string pNameString = player.playerName;
        std::string piece = Utils::pieceTypeToString(player.piece);
        std::string isHost = (player.isHost ? "  HOST" : "");
        text.setString("Name: " + pNameString
                       + "  ID: " + std::to_string(player.playerId)
                       + "  Piece: " + piece
                       + isHost);
        text.move({0, DEFAULT_WIDGET_Y_OFFSET});
        window.draw(text);
    }

    // Render buttons
    for (const auto &[id, btn]: gameRoomButtons) {
        btn->render(window);
    }
}

void GameClient::renderGame() {
    //TODO:
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2i hoveredGridPos = BoardRenderer::getSquareAt(mousePos, boardData, BOARD_DRAW_AREA);

    BoardRenderer::render(window, boardData, BOARD_DRAW_AREA, isMyTurn, hoveredGridPos);

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
                std::string pNameString = gameEndPlayer.playerName;
                std::string winString = "Player[" + std::to_string(gameEndPlayer.playerId) + "] " + pNameString
                + " playing " + Utils::pieceTypeToString(gameEndPlayer.piece) + " has won the round!";
                gameEndText.setString(winString);
                break;
            }

            case FinishReason::PLAYER_DISCONNECT: {
                // Player {} has disconnected
                std::string pNameString = gameEndPlayer.playerName;
                std::string disconnectString = "Player[" + std::to_string(gameEndPlayer.playerId) + "] " + pNameString
                + " playing " + Utils::pieceTypeToString(gameEndPlayer.piece) + " has disconnected!";
                gameEndText.setString(disconnectString);
                break;
            }
        }

        //Center the text
        sf::FloatRect gameEndBounds = gameEndText.getGlobalBounds();
        gameEndText.setOrigin({
            std::floor(gameEndBounds.position.x + gameEndBounds.size.x / 2.0f),
            std::floor(gameEndBounds.position.y + gameEndBounds.size.y / 2.0f)
        });
        gameEndText.setPosition({
            std::floor(WIN_TEXT_DRAW_AREA.position.x + WIN_TEXT_DRAW_AREA.size.x / 2.0f) + 1.0f,
            std::floor(WIN_TEXT_DRAW_AREA.position.y + WIN_TEXT_DRAW_AREA.size.y / 7.0f) - 4.0f
        });
        window.draw(gameEndText);

        //TODO: display current score:
        sf::Text scoreBoardText(font);
        scoreBoardText.setCharacterSize(24);
        scoreBoardText.setFillColor(sf::Color(TEXT_COLOR));
        scoreBoardText.setString("Score Board");
        //Center the text <- pack into a function
        sf::FloatRect scoreBoardBounds = scoreBoardText.getGlobalBounds();
        scoreBoardText.setOrigin({
            std::floor(scoreBoardBounds.position.x + scoreBoardBounds.size.x / 2.0f),
            std::floor(scoreBoardBounds.position.y + scoreBoardBounds.size.y / 2.0f)
        });
        scoreBoardText.setPosition({
            std::floor(WIN_TEXT_DRAW_AREA.position.x + WIN_TEXT_DRAW_AREA.size.x / 2.0f) + 1.0f,
            std::floor(WIN_TEXT_DRAW_AREA.position.y + WIN_TEXT_DRAW_AREA.size.y / 7.0f) - 4.0f + DEFAULT_WIDGET_Y_OFFSET + 8.0f
        });
        window.draw(scoreBoardText);


        int offset = 1;
        for (const auto &player: players) {
            sf::Text scoreText(font);
            scoreText.setCharacterSize(DEFAULT_TEXT_SIZE);
            scoreText.setFillColor(sf::Color(TEXT_COLOR));

            std::string pNameString = player.playerName;
            std::string scoreString = playerName + " " + Utils::pieceTypeToString(player.piece) + " Wins: " + std::to_string(player.wins);
            scoreText.setString(scoreString);

            //Center the text
            sf::FloatRect scoreBounds = scoreText.getGlobalBounds();
            scoreText.setOrigin({
                std::floor(scoreBounds.position.x + scoreBounds.size.x / 2.0f),
                std::floor(scoreBounds.position.y + scoreBounds.size.y / 2.0f)
            });

            scoreText.setPosition({
                std::floor(WIN_TEXT_DRAW_AREA.position.x + WIN_TEXT_DRAW_AREA.size.x / 2.0f) + 1.0f,
                std::floor(WIN_TEXT_DRAW_AREA.position.y + WIN_TEXT_DRAW_AREA.size.y / 7.0f) - 4.0f + DEFAULT_WIDGET_Y_OFFSET + 8.0f + ((DEFAULT_WIDGET_Y_OFFSET - 6) * offset)
            });

            // scoreText.move({0, static_cast<float>(defaultWidgetYOffset * offset)});
            window.draw(scoreText);
            ++offset;
        }

        //TODO: If the client isnt the host draw text with "Waiting for the host"
    }

    for (const auto &btn: gameButtons | std::views::values) {
        btn->render(window);
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

        text.setString("Current Tick: " + std::to_string(serverLogic.getTick()));
        text.move({0, textYOffset});
        window.draw(text);

        //nextplayerid
        text.setString("NextPlayerID: " + std::to_string(serverLogic.getNextPlayerId()));
        text.move({0, textYOffset});
        window.draw(text);

        //turn
        text.setString("Turn: " + std::to_string(serverLogic.getCurrentTurn()));
        text.move({0, textYOffset});
        window.draw(text);

        //hostingplayer
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
        for (const auto &player : serverLogic.getPlayers()) {
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

GameClient::~GameClient() {
    if (hosting) {
        stopInternalServerThread();
    }
}



