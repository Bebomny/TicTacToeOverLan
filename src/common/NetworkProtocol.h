#ifndef TICTACTOEOVERLAN_NETWORKPROTOCOL_H
#define TICTACTOEOVERLAN_NETWORKPROTOCOL_H
#include <cstdint>
#include <winsock2.h>

#include "../common/GameDefinitions.h"

//The websocket version to use, windows requires it to be specified before creating sockets
constexpr static WORD REQ_SOCK_VERSION = MAKEWORD(2, 2);
constexpr static int DEFAULT_BUFFER_LEN = 4096;
constexpr static int MAX_PLAYER_NAME_LENGTH = 32;
constexpr static int MAX_PLAYERS = 6;

/**
 * @brief Identifiers for the specific type of payload contained in a packet.
 * <br> Used by the receiver to cast the payload buffer to the correct struct type after reading the header.
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

// This is so the compiler doesn't mess with the padding in the network logic
// Forces the compiler to align struct members on 1-byte boundaries
// This prevents the compiler from adding padding bytes for optimization, ensuring
// the struct's binary representation is identical across different machines
#pragma pack(push, 1)

/**
 * @brief The Standard Header prepended to ALL network messages.
 * <br> The receiver reads these bytes first to determine how much more data to read.
 */
struct PacketHeader {
  PacketType type;
  uint32_t payloadSize;
};

/**
 * @brief Initial handshake packet.
 * <br> Sent immediately by the server upon socket connection.
 */
struct ServerHelloPacket {
  uint8_t playerId;
};

/**
 * @brief Login request.
 * <br> Sent by the client to finalize the connection setup.
 */
struct SetupReqPacket {
  uint8_t playerId;
  int32_t initialToken; //A client generated token, for later validating moves;
  char playerName[MAX_PLAYER_NAME_LENGTH];
  bool isHost;
};

/**
 * @brief Login acknowledgement.
 * <br> Contains the "Snapshot" of the current lobby state so the new client catches up.
 */
struct SetupAckPacket {
  int32_t generatedAuthToken;
  uint8_t playerId;
  char playerName[MAX_PLAYER_NAME_LENGTH];
  PieceType pieceType;
  uint8_t boardSize;
  uint8_t winConditionLength;
  uint16_t round;
  uint8_t playerCount;
  Player players[MAX_PLAYERS];
};

/**
 * @brief Notification that a new peer has joined.
 */
struct NewPlayerJoinPacket {
  uint8_t newPlayerId;
  PieceType newPlayerPieceType;
  char newPlayerName[MAX_PLAYER_NAME_LENGTH];
  bool isHost;
};

/**
 * @brief Notification that a peer has left.
 */
struct PlayerDisconnectedPacket {
  uint8_t playerId;
};

/**
 * @brief Request to modify game rules.
 * <br> Only valid when in the Lobby/Waiting Room.
 */
struct SettingsChangeReqPacket {
  uint8_t playerId;
  int32_t authToken;
  uint8_t newBoardSize;
  uint8_t newWinConditionLength;
};

/**
 * @brief Confirms that settings have been modified.
 */
struct SettingsUpdatePacket {
  uint8_t newBoardSize;
  uint8_t newWinConditionLength;
};

/**
 * @brief Request to transition from WAITING_ROOM to GAME.
 */
struct GameStartRequestPacket {
  uint8_t requestingPlayerId;
  bool newGame;
};

/**
 * @brief Signal to switch UI to the Board view and initialize grid.
 */
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

/**
 * @brief Event-based full state synchronization.
 * <br> Sent by the server to ensure all clients have the exact same board data.
 * <br> Sent typically after a valid move is processed.
 */
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

/**
 * @brief Client intent to place a piece.
 */
struct MoveRequestPacket {
  uint8_t playerId;
  uint8_t x, y;
  uint16_t turn;
  PieceType piece;
};

/**
 * @brief Signal to return the UI to the Waiting Room.
 */
struct BackToGameRoomPacket {
  uint8_t playerId;
};

/**
 * @brief Signal that the match has reached a conclusion.
 */
struct GameEndPacket {
  FinishReason reason;
  uint8_t playerId;
  Player player;
};

// Restore default compiler structure packing.
#pragma pack(pop)

#endif //TICTACTOEOVERLAN_NETWORKPROTOCOL_H
