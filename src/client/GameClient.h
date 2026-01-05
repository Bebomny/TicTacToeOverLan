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

enum class SetupPhase : uint8_t {
    DISCONNECTED,
    CONN_ESTABLISHED,
    SETTING_UP,
    CONNECTED,
};

enum class GamePhase : uint8_t {
    UNKNOWN,
    WAITING_ROOM,
    MY_TURN,
    NOT_MY_TURN,
    GAME_FINISHED
};

enum class ClientState : uint8_t {
    MENU,
    GAME_ROOM,
    GAME
};

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
    std::map<std::string, std::unique_ptr<Widget>> widgets;
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
    constexpr static sf::FloatRect WIN_TEXT_DRAW_AREA= {{
        BOARD_DRAW_AREA.position.x - 50.0f,
        BOARD_DRAW_AREA.position.y + (BOARD_DRAW_AREA.size.y / 2.0f) - 150.0f},
        {600.0f, 300.0f}
    };

    //Server Connection
    InternalGameServer serverLogic; // The logic object
    std::thread serverThread;   // The handle to the execution
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
    BoardData boardData {};
    uint8_t playerCount;
    std::vector<Player> players;
    std::vector<Move> moves;
    Move lastMove;
    bool isMyTurn = false;
    FinishReason finishReason;
    Player gameEndPlayer;

    GameClient();

    void run();

    ~GameClient();

private:
    void handleInput();
    void update();
    void render();

    void initWidgets();

    void handleMenuInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos);
    void handleGameRoomInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos);
    void handleGameInput(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos);

    void renderMenu();
    void renderGameRoom();
    void renderGame();
    void renderDebugMenu();

    void startInternalServerThread();
    void stopInternalServerThread();

    void connectAndSetup();
    void disconnect();

    void requestBoardSettingsUpdate(uint8_t newBoardSize, uint8_t newWinConditionLength);

    void startGame(bool newGame);

    void sendMove(uint8_t posX, uint8_t posY);
};


#endif //TICTACTOEOVERLAN_GAMECLIENT_H