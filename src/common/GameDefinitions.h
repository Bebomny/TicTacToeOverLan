#ifndef TICTACTOEOVERLAN_GAMEDEFINITIONS_H
#define TICTACTOEOVERLAN_GAMEDEFINITIONS_H
#include <cstdint>
#include <vector>

constexpr static uint8_t MAX_BOARD_SIZE = 32;
constexpr static uint16_t TOTAL_BOARD_AREA = MAX_BOARD_SIZE * MAX_BOARD_SIZE;
constexpr static uint8_t MAX_WIN_CONDITION_LENGTH = 32;

/**
 * The available Piece Types. Used from Top to Bottom, selected by the player number
 */
enum class PieceType : uint8_t {
    EMPTY,
    CROSS,
    CIRCLE,
    TRIANGLE,
    SQUARE,
    DIAMOND,
    OCTAGON
};

struct Player {
    PieceType piece;
    uint8_t playerId;
    uint32_t wins;
    char playerName[32]; //Circular dependencies here move to a variable later
    bool myTurn;
    bool isMe;
    bool isHost;
};

struct BoardSquare {
    PieceType piece;
    uint8_t playerId;
    uint16_t turnPlaced;
};

struct Move {
    PieceType piece;
    uint8_t playerId;
    uint16_t turnPlaced;
    uint8_t posX;
    uint8_t posY;
};

struct BoardData {
    std::vector<std::vector<BoardSquare>> grid;
    uint8_t boardSize;
    uint8_t winConditionLength;
    uint16_t round;
    uint16_t turn;
    uint8_t actingPlayerId;

    BoardSquare getSquareAt(const uint8_t x, const uint8_t y) const {
        return grid.at(y).at(x);
    }

    void setSquareAt(const uint8_t x, const uint8_t y, const BoardSquare square) {
        grid.at(y).at(x) = std::move(square);
    }
};

#endif //TICTACTOEOVERLAN_GAMEDEFINITIONS_H