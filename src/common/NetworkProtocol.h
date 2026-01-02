#ifndef TICTACTOEOVERLAN_NETWORKPROTOCOL_H
#define TICTACTOEOVERLAN_NETWORKPROTOCOL_H
#include <cstdint>
#include <winsock2.h>

#include "../common/GameDefinitions.h"

constexpr static WORD REQ_SOCK_VERSION = MAKEWORD(2, 2);
//The websocket version to use, windows requires it to be specified before creating sockets
constexpr static int DEFAULT_BUFFER_LEN = 4096;
constexpr static int MAX_PLAYER_NAME_LENGTH = 32;
constexpr static int MAX_PLAYERS = 6;

/**
 * The enum containing all packet types
 */
enum class PacketType : uint8_t {
  SERVER_HELLO,
  SETUP_REQ,
  SETUP_ACK,
  NEW_PLAYER_JOIN,
  PLAYER_DISCONNECTED,
  SETTINGS_CHANGE_REQ,
  SETTINGS_UPDATE,
  GAME_START_REQ,
  GAME_START,
  MOVE_REQ,
  BOARD_STATE_UPDATE,
  BACK_TO_GAME_ROOM,
  GAME_END
};

// This is so the compiler doesnt mess with the padding in the network logic
#pragma pack(push, 1)

/**
 * The packet header, so the server/client know how many bytes to read and how to process the data
 */
struct PacketHeader {
  PacketType type;
  uint32_t payloadSize;
};

struct ServerHelloPacket {
  uint8_t playerId;
};

struct SetupReqPacket {
  uint8_t playerId;
  int32_t initialToken; //A client generated token, for later validating moves;
  char playerName[MAX_PLAYER_NAME_LENGTH];
  bool isHost;
};

struct SetupAckPacket {
  int32_t generatedAuthToken;
  uint8_t playerId;
  char playerName[MAX_PLAYER_NAME_LENGTH];
  PieceType pieceType;
  //TODO: send the initial board settings here too
  uint8_t boardSize;
  uint8_t winConditionLength;
  uint16_t round;
  // std::vector<Player> players;
  uint8_t playerCount;
  Player players[MAX_PLAYERS];
};

struct NewPlayerJoinPacket {
  uint8_t newPlayerId;
  PieceType newPlayerPieceType;
  char newPlayerName[MAX_PLAYER_NAME_LENGTH];
  bool isHost;
};

struct PlayerDisconnectedPacket {
  uint8_t playerId;
};

struct SettingsChangeReqPacket {
  uint8_t playerId;
  int32_t authToken;
  uint8_t newBoardSize;
  uint8_t newWinConditionLength;
};

struct SettingsUpdatePacket {
  uint8_t newBoardSize;
  uint8_t newWinConditionLength;
};

struct GameStartRequestPacket {
  uint8_t requestingPlayerId;
  bool newGame;
};

struct GameStartPacket {
  BoardSquare grid[TOTAL_BOARD_AREA];
  uint8_t requestedByPlayerId;
  uint8_t finalBoardSize;
  uint8_t finalWinConditionLength;
  uint8_t round;
  uint8_t turn;
  uint8_t startingPlayerId;
  uint8_t playerCount;
};

struct BoardStateUpdatePacket {
  BoardSquare grid[TOTAL_BOARD_AREA];
  uint8_t boardSize;
  uint8_t winConditionLength;
  uint16_t round;
  uint16_t turn;
  uint8_t actingPlayerId;
  Move lastMove;
  uint8_t playerCount;
  Player players[MAX_PLAYERS];
};

struct MoveRequestPacket {
  uint8_t playerId;
  uint8_t x, y;
  uint16_t turn;
  PieceType piece;
};

struct BackToGameRoomPacket {
  uint8_t playerId;
};

struct GameEndPacket {
  FinishReason reason;
  uint8_t playerId;
  Player player;
};

#pragma pack(pop)

#endif //TICTACTOEOVERLAN_NETWORKPROTOCOL_H
