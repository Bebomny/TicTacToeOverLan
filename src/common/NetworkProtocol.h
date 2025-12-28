#ifndef TICTACTOEOVERLAN_NETWORKPROTOCOL_H
#define TICTACTOEOVERLAN_NETWORKPROTOCOL_H
#include <cstdint>

#include "GameDefinitions.h"

constexpr static WORD REQ_SOCK_VERSION = MAKEWORD(2,2); //The websocket version to use, windows requires it to be specified before creating sockets
constexpr static int DEFAULT_BUFFER_LEN = 4096;

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
  std::string playerName;
};

struct SetupAckPacket {
  int32_t generatedAuthToken;
  uint8_t playerId;
  std::string playerName;
};

struct NewPlayerJoinPacket {
  uint8_t newPlayerId;
  PieceType newPlayerPieceType;
  std::string newPlayerName;
};

struct PlayerDisconnectedPacket {
  uint8_t playerId;
};

struct SettingsChangeReqPacket {
  uint8_t playerId;
  int32_t authToken;
  uint8_t newBoardSize;
  uint8_t winConditionLength;
};

struct BoardStateUpdatePacket {
  BoardData boardData;
  Move lastMove;
  std::vector<Player> players;
};

#pragma pack(pop)

#endif //TICTACTOEOVERLAN_NETWORKPROTOCOL_H