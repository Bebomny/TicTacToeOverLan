#ifndef TICTACTOEOVERLAN_GAMEDEFINITIONS_H
#define TICTACTOEOVERLAN_GAMEDEFINITIONS_H
#include <cstdint>
#include <vector>

constexpr static uint8_t MAX_BOARD_SIZE = 32;
constexpr static uint16_t TOTAL_BOARD_AREA = MAX_BOARD_SIZE * MAX_BOARD_SIZE;
constexpr static uint8_t MAX_WIN_CONDITION_LENGTH = 32;

/**
 * @brief The available Piece Types. Used from Top to Bottom, selected by the player number
 */
enum class PieceType : uint8_t {
    EMPTY,
    CROSS,
    CIRCLE,
    TRIANGLE,
    SQUARE,
    OCTAGON,
    HEXAGON
};

/**
 * @brief Describes why the game session ended.
 */
enum class FinishReason : uint8_t {
    NONE,
    PLAYER_WIN,
    PLAYER_DISCONNECT,
    OTHER
};

/**
 * @brief Represents a participant in the game.
 * <br> Contains identification, statistics, and local state flags.
 */
struct Player {
    PieceType piece;
    uint8_t playerId;
    uint32_t wins;
    char playerName[32];
    bool myTurn;
    bool isMe;
    bool isHost;
};

/**
 * @brief Represents the state of a single cell on the game board.
 */
struct BoardSquare {
    PieceType piece;
    uint8_t playerId;
    uint16_t turnPlaced;
};

/**
 * @brief Represents a discrete action taken by a player.
 * <br> Used for history tracking and network transmission.
 */
struct Move {
    PieceType piece;
    uint8_t playerId;
    uint16_t turnPlaced;
    uint8_t posX;
    uint8_t posY;
};

/**
 * @brief The core data model for the game board.
 * <br> Holds the grid state, rules (size/win condition), and turn counters.
 */
struct BoardData {
    std::vector<std::vector<BoardSquare> > grid;
    uint8_t boardSize;
    uint8_t winConditionLength;
    uint16_t round;
    uint16_t turn;
    uint8_t actingPlayerId;

    /**
     * @brief Accessor for grid cells.
     *
     * @param x The X coordinate (column).
     * @param y The Y coordinate (row).
     * @return A copy of the BoardSquare at the given location.
     */
    BoardSquare getSquareAt(const uint8_t x, const uint8_t y) const {
        return grid.at(y).at(x);
    }

    /**
     * @brief Updates the state of a specific grid cell.
     *
     * @param x The X coordinate (column).
     * @param y The Y coordinate (row).
     * @param square The new state to assign to the cell.
     */
    void setSquareAt(const uint8_t x, const uint8_t y, const BoardSquare square) {
        grid.at(y).at(x) = std::move(square);
    }
};

#endif //TICTACTOEOVERLAN_GAMEDEFINITIONS_H
