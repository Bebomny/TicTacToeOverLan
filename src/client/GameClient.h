#ifndef TICTACTOEOVERLAN_GAMECLIENT_H
#define TICTACTOEOVERLAN_GAMECLIENT_H
#include <cstdint>
#include <map>
#include <regex>
#include <thread>

#include "NetworkManager.h"
#include "../server/InternalGameServer.h"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "ui/ButtonWidget.h"

/**
 * @brief Connection handshake states.
 * <br> Tracks the progress of establishing a connection with the server.
 */
enum class SetupPhase : uint8_t {
    DISCONNECTED,
    CONN_ESTABLISHED,
    SETTING_UP,
    CONNECTED,
};

/**
 * @brief Gameplay flow states.
 * <br> Dictates the logic applied during the actual match.
 */
enum class GamePhase : uint8_t {
    UNKNOWN,
    WAITING_ROOM,
    MY_TURN,
    NOT_MY_TURN,
    GAME_FINISHED
};

/**
 * @brief Application UI states.
 * <br> Determines which screen (Menu, Lobby, Board) is currently rendered and updated.
 */
enum class ClientState : uint8_t {
    MENU,
    GAME_ROOM,
    GAME
};

/**
 * @brief The main application controller.
 * <br> This class manages the entire lifecycle of the client application,
 * including the main game loop (Input -> Update -> Render), window management,
 * networking (client and internal host), and the UI widget registry.
 */
class GameClient {
public:
    SetupPhase setupPhase = SetupPhase::DISCONNECTED;
    GamePhase gamePhase = GamePhase::UNKNOWN;

    NetworkManager networkManager;

    //Menus and windows
    sf::RenderWindow window;
    sf::Clock clock;
    sf::Font font;
    ClientState clientState;
    std::map<std::string, std::unique_ptr<Widget> > widgets;
    bool debugEnabled = true;

    //Menu Coordinates
    constexpr static sf::Vector2f MAIN_MENU_POSITION{36, 36}; //left corner
    constexpr static sf::Vector2f GAME_ROOM_POSITION{36, 36}; //left corner
    constexpr static int DEFAULT_TEXT_SIZE = 20;
    constexpr static int DEFAULT_WIDGET_Y_OFFSET{DEFAULT_TEXT_SIZE + DEFAULT_TEXT_SIZE / 2};
    constexpr static sf::FloatRect BOARD_DRAW_AREA = {
        {300.0f, 60.0f},
        {500.0f, 500.0f}
    };
    constexpr static sf::FloatRect WIN_TEXT_DRAW_AREA = {
        {
            BOARD_DRAW_AREA.position.x - 50.0f,
            BOARD_DRAW_AREA.position.y + (BOARD_DRAW_AREA.size.y / 2.0f) - 150.0f
        },
        {600.0f, 300.0f}
    };

    //Server Connection
    InternalGameServer serverLogic; // The logic object
    std::thread serverThread; // The handle to the execution
    std::string userInputIP;
    std::string serverAddress;
    std::string serverPort;
    bool hosting = false;

    //The player
    uint8_t playerId;
    std::string playerName;
    int32_t initialToken;
    int32_t authToken;
    PieceType pieceType;

    //The board
    BoardData boardData{};
    uint8_t playerCount;
    std::vector<Player> players;
    std::vector<Move> moves;
    Move lastMove;
    bool isMyTurn = false;
    FinishReason finishReason;
    Player gameEndPlayer;

    GameClient();

    ~GameClient();

    /**
     * @brief Main Application Loop.
     * <br> Contains the `while(window.isOpen())` loop.
     * <br> Sequentially calls `handleInput()`, `update()`, and `render()`.
     */
    void run();

private:
    /**
     * @brief Polls SFML events and delegates them to specific state handlers.
     */
    void handleInput();

    /**
     * @brief Processes game logic (network packets, state transitions).
     */
    void update();

    /**
     * @brief Clears the window and calls the appropriate render method for the current ClientState.
     */
    void render();

    /**
     * @brief Initializes all UI widgets (Buttons, TextFields) via their Builders.
     * <br> Called in the constructor, before the main loop starts
     */
    void initWidgets();

    /**
     * @brief Processes input when in the Main Menu.
     *
     * @param event The SFML event.
     * @param mousePos Current mouse position.
     */
    void handleMenuInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos);

    /**
     * @brief Processes input when in the Waiting Room.
     *
     * @param event The SFML event.
     * @param mousePos Current mouse position.
     */
    void handleGameRoomInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos);

    /**
     * @brief Processes input during active gameplay (Board clicks).
     *
     * @param event The SFML event.
     * @param mousePos Current mouse position.
     */
    void handleGameInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos);

    /**
     * @brief Processes the SERVER_HELLO packet.
     *
     * @param packet The parsed ServeHelloPacket packet
     */
    void handleServerHelloPacket(const ServerHelloPacket *packet);

    /**
     * @brief Processes the SETUP_ACK packet.
     *
     * @param packet The parsed SetupAckPacket packet
     */
    void handleSetupAckPacket(const SetupAckPacket *packet);

    /**
     * @brief Processes the NEW_PLAYER_JOIN packet.
     *
     * @param packet The parsed NewPlayerJoinPacket packet
     */
    void handleNewPlayerJoinPacket(const NewPlayerJoinPacket *packet);

    /**
     * @brief Processes the SETTINGS_UPDATE packet.
     *
     * @param packet The parsed SettingsUpdatePacket packet
     */
    void handleSettingsUpdatePacket(const SettingsUpdatePacket *packet);

    /**
     * @brief Processes the PLAYER_DISCONNECTED packet.
     *
     * @param packet The parsed PlayerDisconnectedPacket packet
     */
    void handlePlayerDisconnectedPacket(const PlayerDisconnectedPacket *packet);

    /**
     * @brief Processes the GAME_START packet.
     *
     * @param packet The parsed GameStartPacket packet
     */
    void handleGameStartPacket(const GameStartPacket *packet);

    /**
     * @brief Processes the BOARD_STATE_UPDATE packet.
     *
     * @param packet The parsed BoardStateUpdatePacket packet
     */
    bool handleBoardStateUpdatePacket(const BoardStateUpdatePacket *packet);

    /**
     * @brief Processes the GAME_END packet.
     *
     * @param packet The parsed GameEndPacket packet
     */
    void handleGameEndPacket(const GameEndPacket *packet);

    void renderMenu();

    void renderGameRoom();

    void renderGame();

    /**
     * @brief Overlays debug information (FPS, network stats, etc.) in a separate menu.
     */
    void renderDebugMenu();

    /**
     * @brief Spins up the internal server instance in a separate thread.
     * <br> Called when the user chooses "Host Game".
     */
    void startInternalServerThread();

    /**
     * @brief Signals the internal server to stop.
     */
    void stopInternalServerThread();

    /**
     * @brief Initiates connection to the server (local or remote).
     * <br> Transitions SetupPhase from DISCONNECTED to SETTING_UP.
     */
    void connectAndSetup();

    /**
     * @brief Disconnects from the server and resets networking state.
     */
    void disconnect();

    /**
     * @brief Sends a request to the server to change game rules (Waiting Room Phase).
     *
     * @param newBoardSize The requested grid dimension.
     * @param newWinConditionLength The number of tokens in a row needed to win.
     */
    void requestBoardSettingsUpdate(uint8_t newBoardSize, uint8_t newWinConditionLength);

    /**
     * @brief Sends a start request packet to the server.
     *
     * @param newGame If true, resets player wins and data on the server.
     */
    void startGame(bool newGame);

    /**
     * @brief Transmits a move action to the server.
     *
     * @param posX Grid X coordinate.
     * @param posY Grid Y coordinate.
     */
    void sendMove(uint8_t posX, uint8_t posY);
};


#endif //TICTACTOEOVERLAN_GAMECLIENT_H
