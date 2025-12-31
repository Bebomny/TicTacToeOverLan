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
};

struct SetupAckPacket {
  int32_t generatedAuthToken;
  uint8_t playerId;
  char playerName[MAX_PLAYER_NAME_LENGTH];
  PieceType pieceType;
  //TODO: send the initial board settings here too
  uint8_t boardSize;
  uint8_t winConditionLength;
  // std::vector<Player> players;
  uint8_t playerCount;
  Player players[MAX_PLAYERS];
};

struct NewPlayerJoinPacket {
  uint8_t newPlayerId;
  PieceType newPlayerPieceType;
  char newPlayerName[MAX_PLAYER_NAME_LENGTH];
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

struct BoardStateUpdatePacket {
  BoardData boardData;
  Move lastMove;
  Player players[MAX_PLAYERS];
};

#pragma pack(pop)

#endif //TICTACTOEOVERLAN_NETWORKPROTOCOL_H
