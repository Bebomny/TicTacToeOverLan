#ifndef TICTACTOEOVERLAN_GAMECLIENT_H
#define TICTACTOEOVERLAN_GAMECLIENT_H
#include <cstdint>
#include <thread>

#include "NetworkManager.h"
#include "../server/InternalGameServer.h"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

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
    bool debugEnabled = true; //TODO:make this switch via a button press

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

    void handleMenuInput(const std::optional<sf::Event> &event);
    void handleGameRoomInput(const std::optional<sf::Event> &event);
    void handleGameInput(const std::optional<sf::Event> &event);

    void renderMenu();
    void renderGameRoom();
    void renderGame();
    void renderDebugMenu();

    void startInternalServerThread();
    void stopInternalServerThread();

    void initializeGameBoard();

    void sendMove(uint8_t posX, uint8_t posY);
};


#endif //TICTACTOEOVERLAN_GAMECLIENT_H