#ifndef TICTACTOEOVERLAN_GAMECLIENT_H
#define TICTACTOEOVERLAN_GAMECLIENT_H
#include <cstdint>
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
    sf::Font font;
    ClientState clientState;
    std::vector<std::unique_ptr<ButtonWidget>> menuButtons;
    bool debugEnabled = true;

    //Menu Coordinates - move to separate screen(view?) objects later
    constexpr static sf::Vector2f mainMenuPosition{36, 36}; //left corner
    constexpr static int menuTextSize = 20;
    constexpr static int menuTextYOffset{menuTextSize + menuTextSize / 2};

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
    bool isMyTurn = false;

    GameClient();

    void run();

    ~GameClient();

private:
    void handleInput();
    void update();
    void render();

    void initMenuWidgets();
    void initGameRoomWidgets();
    void initGameWidgets();

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

    void initializeGameBoard();

    void sendMove(uint8_t posX, uint8_t posY);
};


#endif //TICTACTOEOVERLAN_GAMECLIENT_H